#include "ElementEditor.h"
#include "Translation.h"
#include "ElementSelection.h"
#include "QtCompat.h"
#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QDateTimeEdit>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QElapsedTimer>
#include <QFileDialog>
#include <QFontComboBox>
#include <QFormLayout>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QScreen>
#include <QScrollArea>
#include <QShortcut>
#include <QSpinBox>
#include <QSplitter>
#include <QTimer>
#include <QUuid>
#include <QVBoxLayout>
#include <algorithm>
#include <functional>
namespace tt {
namespace {
const QStringList kinds={"clock","current","next","nextTime","countdown","duty","note","today",
    "progress","week","weather","news","dailyWord","progressBar","analogClock","weatherBlock"};
QStringList displayNames() { return {tt::translate("时间"),tt::translate("当前课程"),tt::translate("下一节课"),
    tt::translate("距离下一节"),tt::translate("倒计日"),tt::translate("每日信息"),
    tt::translate("固定文本"),tt::translate("今日课表"),tt::translate("上课进度"),tt::translate("教学周"),
    tt::translate("高德天气"),tt::translate("新闻 / RSS"),tt::translate("每日一句"),
    tt::translate("课程进度条"),tt::translate("模拟时钟"),tt::translate("天气卡片")}; }

class Preview final:public QWidget {
public:
    Preview(QList<ElementDefinition>& elements,const ScheduleStore& store,QWidget* parent=nullptr)
        :QWidget(parent),elements_(elements),store_(store) {
        setObjectName("elementPreview");
        setMinimumSize(320,220);setMouseTracking(true);
        auto* timer=new QTimer(this);timer->setTimerType(Qt::PreciseTimer);
        QObject::connect(timer,&QTimer::timeout,this,QOverload<>::of(&QWidget::update));
        timer->start(ElementLayout::AnimationIntervalMs);clock_.start();
    }
    const QSet<int>& selection() const{return selected_;}
    QList<ElementDefinition> geometry() const{return effectiveGeometry(elements_);}
    void setSelection(QSet<int> selected) {selected_=SelectionGeometry::expanded(elements_,selected);update();}
    void setCanvasSize(QSize size) {canvas_=size;update();}
    void setDark(bool dark) {dark_=dark;update();}
    void setSnapping(int grid,bool edges) {grid_=grid;edges_=edges;}
    std::function<void(int)> selectionChanged;
    std::function<void()> moving;
    std::function<void(const QList<ElementDefinition>&)> moveFinished;
protected:
    QList<ElementDefinition> effectiveGeometry(QList<ElementDefinition> elements) const {
        const auto now=QDateTime::currentDateTime();
        for(auto& element:elements)
            if(element.autoSize)element.rect=ElementLayout::contentBounds({element},store_,now);
        return elements;
    }
    QRect canvasRect() const {
        const QSize scaled=canvas_.scaled(size()-QSize(24,24),Qt::KeepAspectRatio);
        return QRect(QPoint((width()-scaled.width())/2,(height()-scaled.height())/2),scaled);
    }
    QPoint toModel(const QPoint& point) const {
        const QRect r=canvasRect();
        if(r.width()<=0||r.height()<=0)return {};
        return QPoint((point.x()-r.x())*canvas_.width()/r.width(),
                      (point.y()-r.y())*canvas_.height()/r.height());
    }
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);p.setRenderHint(QPainter::Antialiasing);
        p.fillRect(rect(),palette().color(QPalette::Base));
        const QRect viewport=canvasRect();
        p.fillRect(viewport,dark_?QColor("#202535"):QColor("#FBFCFF"));
        p.setPen(QPen(QColor("#ACB3C5"),1));p.drawRect(viewport);
        if(viewport.width()<=0||viewport.height()<=0)return;
        p.save();p.setClipRect(viewport);p.translate(viewport.topLeft());
        p.scale(double(viewport.width())/canvas_.width(),double(viewport.height())/canvas_.height());
        if(grid_>0) {
            p.setPen(QPen(dark_?QColor(255,255,255,28):QColor(60,70,90,30),0));
            for(int x=0;x<canvas_.width();x+=grid_)
                for(int y=0;y<canvas_.height();y+=grid_)p.drawPoint(x,y);
        }
        ElementLayout::paint(p,QRect(QPoint(0,0),canvas_),elements_,store_,
                             QDateTime::currentDateTime(),clock_.elapsed(),dark_);
        const auto geometry=effectiveGeometry(elements_);
        for(int i=0;i<elements_.size();++i) {
            p.setPen(QPen(selected_.contains(i)?QColor("#625DE1"):QColor("#93A1C8"),
                          selected_.contains(i)?2:1,Qt::DashLine));
            p.setBrush(Qt::NoBrush);p.drawRect(geometry[i].rect);
        }
        if(selected_.size()>1) {
            p.setPen(QPen(QColor("#625DE1"),1,Qt::DotLine));
            p.drawRect(SelectionGeometry::bounds(geometry,selected_));
        }
        if(boxSelecting_) {
            p.setPen(QPen(QColor("#625DE1"),1));p.setBrush(QColor(98,93,225,32));
            p.drawRect(QRect(down_,current_).normalized());
        }
        p.restore();
    }
    void mousePressEvent(QMouseEvent* event) override {
        if(event->button()!=Qt::LeftButton)return;
        const QPoint local=mouseLocalPoint(event).toPoint();
        if(!canvasRect().contains(local))return;
        down_=current_=toModel(local);
        const bool control=event->modifiers().testFlag(Qt::ControlModifier);
        const auto geometry=effectiveGeometry(elements_);
        int hit=-1;
        for(int i=elements_.size()-1;i>=0;--i)
            if(geometry[i].rect.contains(down_)){hit=i;break;}
        if(hit>=0) {
            const auto unit=SelectionGeometry::expanded(elements_,QSet<int>{hit});
            if(control) {
                if(selected_.contains(hit))selected_.subtract(unit);else selected_.unite(unit);
            } else if(!selected_.contains(hit))selected_=unit;
            if(selectionChanged)selectionChanged(selected_.contains(hit)?hit:-1);
            dragging_=selected_.contains(hit);
            original_=elements_;originalGeometry_=geometry;
        } else {
            boxBase_=control?selected_:QSet<int>{};
            selected_=boxBase_;boxSelecting_=true;
            if(selectionChanged)selectionChanged(-1);
        }
        update();
    }
    void mouseMoveEvent(QMouseEvent* event) override {
        current_=toModel(mouseLocalPoint(event).toPoint());
        if(dragging_) {
            elements_=original_;
            const QPoint delta=SelectionGeometry::snappedDelta(originalGeometry_,selected_,current_-down_,grid_,edges_,6,
                                                                QRect(QPoint(0,0),canvas_));
            SelectionGeometry::move(elements_,selected_,delta);
            if(moving)moving();
        } else if(boxSelecting_) {
            selected_=boxBase_;
            selected_.unite(SelectionGeometry::inRectangle(effectiveGeometry(elements_),QRect(down_,current_).normalized()));
            if(selectionChanged)selectionChanged(-1);
        } else return;
        update();
    }
    void mouseReleaseEvent(QMouseEvent* event) override {
        if(event->button()!=Qt::LeftButton)return;
        if(dragging_&&moveFinished)moveFinished(original_);
        dragging_=boxSelecting_=false;update();
    }
private:
    QList<ElementDefinition>& elements_;
    const ScheduleStore& store_;
    QList<ElementDefinition> original_,originalGeometry_;
    QElapsedTimer clock_;
    QSize canvas_{420,180};
    QSet<int> selected_,boxBase_;
    bool dark_=false,dragging_=false,boxSelecting_=false,edges_=true;
    int grid_=8;
    QPoint down_,current_;
};
} // namespace

bool ElementEditor::edit(QWidget* parent,QList<ElementDefinition>& output,
                         const ScheduleStore& schedule,QSize size,bool dark) {
    QList<ElementDefinition> working=output;
    QDialog dialog(parent);dialog.setObjectName("elementEditorDialog");
    dialog.setWindowTitle(tt::translate("信息元素布局编辑器"));
    const QRect screen=QGuiApplication::primaryScreen()?QGuiApplication::primaryScreen()->availableGeometry():QRect(0,0,1280,800);
    dialog.resize(std::min(1150,screen.width()-40),std::min(720,screen.height()-40));
    dialog.setMinimumSize(std::min(850,screen.width()-40),std::min(480,screen.height()-40));
    auto* root=new QVBoxLayout(&dialog);
    auto* info=new QLabel(tt::translate("Ctrl 点击多选；空白处拖动框选；拖动选区移动。成组元素一起选择和移动，Ctrl+Z 撤销，Ctrl+Shift+Z 重做。"));
    info->setWordWrap(true);root->addWidget(info);
    auto* tools=new QHBoxLayout;
    auto* undo=new QPushButton(tt::translate("撤销"));undo->setObjectName("editorUndo");
    auto* redo=new QPushButton(tt::translate("重做"));redo->setObjectName("editorRedo");
    auto* group=new QPushButton(tt::translate("组合"));group->setObjectName("editorGroup");
    auto* ungroup=new QPushButton(tt::translate("解组"));ungroup->setObjectName("editorUngroup");
    auto* align=new QComboBox;align->setObjectName("editorAlignment");
    align->addItems({tt::translate("左对齐"),tt::translate("水平居中"),tt::translate("右对齐"),
                    tt::translate("顶部对齐"),tt::translate("垂直居中"),tt::translate("底部对齐")});
    auto* alignButton=new QPushButton(tt::translate("对齐选区"));alignButton->setObjectName("editorAlign");
    for(auto* button:{undo,redo,group,ungroup})tools->addWidget(button);
    tools->addWidget(align);tools->addWidget(alignButton);tools->addStretch();root->addLayout(tools);
    auto* snapping=new QHBoxLayout;
    auto* grid=new QCheckBox(tt::translate("网格吸附"));grid->setChecked(true);grid->setObjectName("editorGrid");
    auto* gridSize=new QSpinBox;gridSize->setObjectName("editorGridSize");gridSize->setRange(1,64);gridSize->setValue(8);gridSize->setSuffix(" px");
    auto* edges=new QCheckBox(tt::translate("元素边缘 / 中心吸附"));edges->setObjectName("editorEdgeSnap");edges->setChecked(true);
    auto* selectionLabel=new QLabel(tt::translate("已选择 0 个元素"));
    snapping->addWidget(grid);snapping->addWidget(gridSize);snapping->addWidget(edges);
    snapping->addStretch();snapping->addWidget(selectionLabel);root->addLayout(snapping);

    auto* split=new QSplitter(Qt::Horizontal,&dialog);
    auto* left=new QWidget;auto* leftCol=new QVBoxLayout(left);
    auto* list=new QListWidget;list->setObjectName("editorElements");
    list->setSelectionMode(QAbstractItemView::ExtendedSelection);list->setMaximumWidth(230);
    auto* actions=new QHBoxLayout;
    auto* add=new QPushButton(tt::translate("新增"));add->setObjectName("editorAdd");
    auto* remove=new QPushButton(tt::translate("删除"));remove->setObjectName("editorRemove");
    actions->addWidget(add);actions->addWidget(remove);
    leftCol->addWidget(new QLabel(tt::translate("显示元素")));leftCol->addWidget(list,1);leftCol->addLayout(actions);
    split->addWidget(left);
    auto* center=new QWidget;auto* centerCol=new QVBoxLayout(center);
    auto* preview=new Preview(working,schedule);
    preview->setCanvasSize(QSize(std::clamp(size.width(),240,1200),std::clamp(size.height(),100,700)));
    preview->setDark(dark);
    centerCol->addWidget(new QLabel(tt::translate("桌面卡片实时预览")));centerCol->addWidget(preview,1);
    split->addWidget(center);

    auto* right=new QWidget;right->setObjectName("editorProperties");
    auto* form=new QFormLayout(right);form->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    auto* kind=new QComboBox;kind->setObjectName("editorKind");
    for(int i=0;i<kinds.size();++i)kind->addItem(displayNames()[i],kinds[i]);
    auto* mode=new QComboBox;mode->setObjectName("editorMode");
    mode->addItem(tt::translate("单一元素"),"single");mode->addItem(tt::translate("轮播"),"cycle");
    mode->addItem(tt::translate("同时显示"),"all");
    auto* allLayout=new QComboBox;allLayout->setObjectName("editorAllLayout");
    allLayout->addItem(tt::translate("纵向"),"vertical");allLayout->addItem(tt::translate("横向"),"horizontal");
    allLayout->addItem(tt::translate("网格"),"grid");
    auto* columns=new QSpinBox;columns->setObjectName("editorColumns");columns->setRange(1,16);
    auto* itemIndex=new QSpinBox;itemIndex->setObjectName("editorItemIndex");itemIndex->setRange(1,1);
    auto* addText=new QPushButton(tt::translate("增加条目"));addText->setObjectName("editorAddText");
    auto* removeText=new QPushButton(tt::translate("移除条目"));removeText->setObjectName("editorRemoveText");
    auto* format=new QLineEdit;format->setObjectName("editorFormat");
    format->setPlaceholderText(tt::translate("%H:%M:%S 或「当前：{value}」"));
    auto* font=new QFontComboBox;
    auto* fontSize=new QSpinBox;fontSize->setRange(7,64);
    auto* bold=new QCheckBox(tt::translate("粗体"));
    auto* color=new QPushButton(tt::translate("设置文字颜色"));
    auto* x=new QSpinBox;x->setObjectName("editorX");auto* y=new QSpinBox;y->setObjectName("editorY");
    for(auto* v:{x,y})v->setRange(-2048,4096);
    auto* w=new QSpinBox;w->setObjectName("editorWidth");auto* h=new QSpinBox;h->setObjectName("editorHeight");
    for(auto* v:{w,h})v->setRange(16,3000);
    auto* autoSize=new QCheckBox(tt::translate("按内容自动计算尺寸"));autoSize->setObjectName("editorAutoSize");
    auto* interval=new QSpinBox;interval->setRange(1,600);interval->setSuffix(tt::translate(" 秒"));
    auto* cycleAfterScroll=new QCheckBox(tt::translate("长文字滚完再切换"));cycleAfterScroll->setObjectName("editorCycleAfterScroll");
    auto* marquee=new QCheckBox(tt::translate("超宽文字滚动"));
    auto* speed=new QDoubleSpinBox;speed->setObjectName("editorScrollSpeed");speed->setRange(1,1000);speed->setDecimals(1);speed->setSuffix(" px/s");
    auto* rotateDuty=new QCheckBox(tt::translate("每日信息逐条轮换"));rotateDuty->setObjectName("editorRotateDuty");
    auto* vertical=new QCheckBox(tt::translate("今日课表按竖列显示"));
    auto* abbreviate=new QCheckBox(tt::translate("今日课表只显示课程首字"));
    auto* weatherKey=new QLineEdit;weatherKey->setEchoMode(QLineEdit::Password);
    weatherKey->setPlaceholderText(tt::translate("高德 Web 服务 API Key（本机保存）"));
    auto* weatherCode=new QLineEdit;weatherCode->setPlaceholderText(tt::translate("区划代码，如 110000"));
    auto* weatherCity=new QLineEdit;weatherCity->setPlaceholderText(tt::translate("城市备注，可选"));
    auto* weatherAddress=new QLineEdit;weatherAddress->setObjectName("editorWeatherAddress");weatherAddress->setPlaceholderText(tt::translate("Adcode 留空时识别，如北京市海淀区"));
    auto* weatherForecast=new QCheckBox(tt::translate("显示天气预报"));weatherForecast->setObjectName("editorWeatherForecast");
    auto* sourceUrl=new QLineEdit;sourceUrl->setObjectName("editorSourceUrl");sourceUrl->setPlaceholderText(tt::translate("HTTPS RSS / Atom / JSON 地址"));
    auto* sourceFile=new QLineEdit;sourceFile->setObjectName("editorSourceFile");sourceFile->setPlaceholderText(tt::translate("UTF-8 文本 / RSS / JSON 文件"));
    auto* browse=new QPushButton(tt::translate("浏览…"));
    auto* sourceRow=new QWidget;auto* sourceLayout=new QHBoxLayout(sourceRow);sourceLayout->setContentsMargins(0,0,0,0);
    sourceLayout->addWidget(sourceFile,1);sourceLayout->addWidget(browse);
    auto* sourceHint=new QLabel(tt::translate("新闻和每日一句支持 HTTPS RSS / JSON 或 UTF-8 文件。本机文件优先；源留空时可在「文本 / 格式」填写固定内容。"));
    sourceHint->setWordWrap(true);
    auto* target=new QDateTimeEdit(QDateTime::currentDateTime().addDays(30));
    target->setDisplayFormat("yyyy-MM-dd HH:mm:ss");target->setCalendarPopup(true);
    form->addRow(tt::translate("内容类型"),kind);form->addRow(tt::translate("组合模式"),mode);
    form->addRow(tt::translate("同时排布"),allLayout);form->addRow(tt::translate("网格列数"),columns);
    form->addRow(tt::translate("条目编号"),itemIndex);form->addRow(addText);form->addRow(removeText);
    form->addRow(tt::translate("文本 / 格式"),format);form->addRow(tt::translate("字体"),font);
    form->addRow(tt::translate("字号"),fontSize);form->addRow(bold);form->addRow(color);
    form->addRow(QStringLiteral("X"),x);form->addRow(QStringLiteral("Y"),y);
    form->addRow(tt::translate("宽度"),w);form->addRow(tt::translate("高度"),h);form->addRow(autoSize);
    form->addRow(tt::translate("轮播间隔"),interval);form->addRow(cycleAfterScroll);form->addRow(marquee);
    form->addRow(tt::translate("滚动速度"),speed);form->addRow(rotateDuty);form->addRow(vertical);form->addRow(abbreviate);
    form->addRow(tt::translate("倒计目标"),target);form->addRow(tt::translate("天气 Key"),weatherKey);
    form->addRow(tt::translate("高德 Adcode"),weatherCode);form->addRow(tt::translate("城市名称"),weatherCity);
    form->addRow(tt::translate("识别地址"),weatherAddress);form->addRow(weatherForecast);
    form->addRow(tt::translate("订阅地址"),sourceUrl);form->addRow(tt::translate("文本文件"),sourceRow);form->addRow(sourceHint);
    auto* propertyScroll=new QScrollArea;propertyScroll->setObjectName("editorPropertyScroll");
    propertyScroll->setWidgetResizable(true);propertyScroll->setWidget(right);propertyScroll->setMinimumWidth(260);
    split->addWidget(propertyScroll);
    split->setStretchFactor(0,0);split->setStretchFactor(1,2);split->setStretchFactor(2,1);
    split->setSizes({180,550,330});root->addWidget(split,1);
    auto* buttons=new QDialogButtonBox(QDialogButtonBox::Save|QDialogButtonBox::Cancel);
    buttons->setObjectName("editorButtons");buttons->button(QDialogButtonBox::Save)->setText(tt::translate("保存布局"));
    root->addWidget(buttons);

    bool updating=false;
    struct State {QList<ElementDefinition> elements;QSet<int> selected;int current=-1;};
    QList<State> history;int historyPosition=0;
    std::function<void()> refreshFields,refreshList,refreshActions;
    auto currentState=[&] {return State{working,preview->selection(),list->currentRow()};};
    auto selected=[&]()->ElementDefinition* {
        const int n=list->currentRow();return n>=0&&n<working.size()?&working[n]:nullptr;
    };
    auto selectedText=[&]()->TextElement* {
        auto* element=selected();if(!element)return nullptr;
        const int n=itemIndex->value()-1;return n>=0&&n<element->items.size()?&element->items[n]:nullptr;
    };
    auto labelFor=[&](int i) {
        const auto& e=working[i];const QString type=e.items.isEmpty()?tt::translate("空元素"):e.items.first().type;
        const int kindIndex=kinds.indexOf(type);
        return QStringLiteral("%1. %2%3 (%4,%5)").arg(i+1)
            .arg(kindIndex<0?type:displayNames()[kindIndex]).arg(e.groupId.isEmpty()?QString():tt::translate(" [组]"))
            .arg(e.rect.x()).arg(e.rect.y());
    };
    refreshActions=[&] {
        undo->setEnabled(historyPosition>0);redo->setEnabled(historyPosition+1<history.size());
        const auto selection=preview->selection();group->setEnabled(selection.size()>1);
        bool grouped=false;for(int i:selection)if(i>=0&&i<working.size()&&!working[i].groupId.isEmpty())grouped=true;
        ungroup->setEnabled(grouped);alignButton->setEnabled(selection.size()>1);remove->setEnabled(!selection.isEmpty());
        selectionLabel->setText(tt::translate("已选择 %1 个元素").arg(selection.size()));
    };
    refreshFields=[&] {
        if(updating)return;
        updating=true;right->setEnabled(selected()!=nullptr);
        if(auto* e=selected()) {
            x->setValue(e->rect.x());y->setValue(e->rect.y());w->setValue(e->rect.width());h->setValue(e->rect.height());
            mode->setCurrentIndex(std::max(0,mode->findData(e->mode)));
            allLayout->setCurrentIndex(std::max(0,allLayout->findData(e->allLayout)));columns->setValue(e->columns);
            allLayout->setEnabled(e->mode=="all");columns->setEnabled(e->mode=="all"&&e->allLayout=="grid");
            interval->setValue(e->intervalSeconds);marquee->setChecked(e->marquee);autoSize->setChecked(e->autoSize);
            w->setEnabled(!e->autoSize);h->setEnabled(!e->autoSize);
            cycleAfterScroll->setChecked(e->cycleAfterScroll);cycleAfterScroll->setEnabled(e->mode=="cycle");
            speed->setValue(e->scrollPixelsPerSecond);speed->setEnabled(e->marquee);
            itemIndex->setMaximum(std::max(1,int(e->items.size())));
            if(itemIndex->value()>e->items.size())itemIndex->setValue(1);
            removeText->setEnabled(e->items.size()>1);
            if(auto* t=selectedText()) {
                kind->setCurrentIndex(std::max(0,kind->findData(t->type)));format->setText(t->format);
                font->setCurrentFont(QFont(t->font));fontSize->setValue(t->pointSize);bold->setChecked(t->bold);
                vertical->setChecked(t->vertical);abbreviate->setChecked(t->abbreviate);rotateDuty->setChecked(t->rotateDuty);
                vertical->setEnabled(t->type=="today");abbreviate->setEnabled(t->type=="today");rotateDuty->setEnabled(t->type=="duty");
                weatherKey->setText(t->weatherApiKey);weatherCode->setText(t->weatherAdcode);weatherCity->setText(t->weatherCity);
                weatherAddress->setText(t->weatherAddress);weatherForecast->setChecked(t->weatherForecast);
                const bool weather=t->type=="weather"||t->type=="weatherBlock";
                for(auto* field:{weatherKey,weatherCode,weatherCity,weatherAddress})field->setEnabled(weather);
                weatherForecast->setEnabled(weather);sourceUrl->setText(t->sourceUrl);sourceFile->setText(t->sourceFile);
                sourceUrl->setEnabled(t->type=="news"||t->type=="dailyWord");sourceRow->setEnabled(t->type=="news"||t->type=="dailyWord");
                sourceHint->setEnabled(sourceRow->isEnabled());target->setEnabled(t->type=="countdown");
                color->setStyleSheet("background-color:"+t->color.name()+";color:"+(t->color.lightness()<128?"white":"black")+";");
                target->setDateTime(t->target.isValid()?t->target:QDateTime::currentDateTime().addDays(30));
            }
        }
        updating=false;refreshActions();preview->update();
    };
    refreshList=[&] {
        const int current=list->currentRow();const auto selection=preview->selection();
        updating=true;list->clear();
        for(int i=0;i<working.size();++i)list->addItem(labelFor(i));
        if(!working.isEmpty())list->setCurrentRow(std::clamp(current,0,int(working.size())-1),QItemSelectionModel::NoUpdate);
        for(int i:selection)if(i>=0&&i<list->count())list->item(i)->setSelected(true);
        updating=false;refreshFields();
    };
    auto record=[&](const State& before) {
        if(ElementLayout::toJson(before.elements)==ElementLayout::toJson(working))return;
        if(history.isEmpty()) {history<<before;historyPosition=0;}
        // Update the preceding state's selection to the selection used for this action.
        history[historyPosition].selected=before.selected;history[historyPosition].current=before.current;
        while(history.size()>historyPosition+1)history.removeLast();
        history<<currentState();historyPosition=history.size()-1;
        if(history.size()>100){history.removeFirst();--historyPosition;}
        refreshActions();
    };
    auto restore=[&](int position) {
        if(position<0||position>=history.size())return;
        historyPosition=position;const State state=history[position];working=state.elements;
        preview->setSelection(state.selected);refreshList();
        updating=true;list->setCurrentRow(state.current,QItemSelectionModel::NoUpdate);updating=false;
        refreshFields();
    };
    auto synchronizeSelection=[&](QSet<int> selection,int current) {
        selection=SelectionGeometry::expanded(working,selection);preview->setSelection(selection);
        updating=true;list->clearSelection();
        for(int i:selection)if(i>=0&&i<list->count())list->item(i)->setSelected(true);
        if(current<0||!selection.contains(current))current=selection.isEmpty()?-1:*std::min_element(selection.begin(),selection.end());
        list->setCurrentRow(current,QItemSelectionModel::NoUpdate);updating=false;refreshFields();
    };
    QObject::connect(list,&QListWidget::itemSelectionChanged,&dialog,[&] {
        if(updating)return;QSet<int> selection;
        for(auto* item:list->selectedItems())selection.insert(list->row(item));
        synchronizeSelection(selection,list->currentRow());
    });
    QObject::connect(list,&QListWidget::currentRowChanged,&dialog,[&](int){if(!updating)refreshFields();});
    preview->selectionChanged=[&](int current){synchronizeSelection(preview->selection(),current);};
    preview->moving=[&] {
        updating=true;if(auto* e=selected()){x->setValue(e->rect.x());y->setValue(e->rect.y());}
        for(int i=0;i<working.size();++i)if(list->item(i))list->item(i)->setText(labelFor(i));updating=false;
    };
    preview->moveFinished=[&](const QList<ElementDefinition>& before) {
        record(State{before,preview->selection(),list->currentRow()});refreshFields();
    };
    auto updateSnapping=[&] {preview->setSnapping(grid->isChecked()?gridSize->value():0,edges->isChecked());preview->update();};
    QObject::connect(grid,&QCheckBox::toggled,&dialog,[&](bool){updateSnapping();});
    QObject::connect(edges,&QCheckBox::toggled,&dialog,[&](bool){updateSnapping();});
    QObject::connect(gridSize,QOverload<int>::of(&QSpinBox::valueChanged),&dialog,[&](int){updateSnapping();});
    QObject::connect(undo,&QPushButton::clicked,&dialog,[&]{restore(historyPosition-1);});
    QObject::connect(redo,&QPushButton::clicked,&dialog,[&]{restore(historyPosition+1);});
    auto shortcut=[&](QKeySequence keys,QPushButton* button) {
        auto* action=new QShortcut(keys,&dialog);QObject::connect(action,&QShortcut::activated,button,&QPushButton::click);
    };
    shortcut(QKeySequence::Undo,undo);shortcut(QKeySequence("Ctrl+Shift+Z"),redo);shortcut(QKeySequence("Ctrl+Y"),redo);
    shortcut(QKeySequence("Ctrl+G"),group);shortcut(QKeySequence("Ctrl+Shift+G"),ungroup);
    QObject::connect(group,&QPushButton::clicked,&dialog,[&] {
        if(preview->selection().size()<2)return;const State before=currentState();
        const QString id=QUuid::createUuid().toString(QUuid::WithoutBraces);
        for(int i:preview->selection())working[i].groupId=id;
        record(before);refreshList();
    });
    QObject::connect(ungroup,&QPushButton::clicked,&dialog,[&] {
        const State before=currentState();for(int i:preview->selection())working[i].groupId.clear();record(before);refreshList();
    });
    QObject::connect(alignButton,&QPushButton::clicked,&dialog,[&] {
        const State before=currentState();auto geometry=preview->geometry();
        SelectionGeometry::align(geometry,preview->selection(),
            static_cast<SelectionGeometry::Alignment>(align->currentIndex()));
        for(int i:preview->selection())working[i].rect.moveTopLeft(geometry[i].rect.topLeft());
        record(before);refreshList();
    });
    QObject::connect(itemIndex,QOverload<int>::of(&QSpinBox::valueChanged),&dialog,[&](int){if(!updating)refreshFields();});
    QObject::connect(add,&QPushButton::clicked,&dialog,[&] {
        const State before=currentState();ElementDefinition e;
        e.rect=QRect(10+int(working.size())*12,10+int(working.size())*12,170,30);
        TextElement t;t.type="clock";t.format="%H:%M:%S";e.items<<t;working<<e;
        preview->setSelection({int(working.size())-1});refreshList();
        synchronizeSelection(preview->selection(),working.size()-1);record(before);
    });
    QObject::connect(remove,&QPushButton::clicked,&dialog,[&] {
        const State before=currentState();QList<int> indices=preview->selection().values();std::sort(indices.begin(),indices.end(),std::greater<int>());
        for(int i:indices)working.removeAt(i);preview->setSelection({});refreshList();
        if(!working.isEmpty())synchronizeSelection({std::clamp(before.current,0,int(working.size())-1)},-1);
        record(before);
    });
    QObject::connect(addText,&QPushButton::clicked,&dialog,[&] {
        const State before=currentState();
        if(auto* e=selected()) {e->items<<TextElement{};
            updating=true;itemIndex->setMaximum(e->items.size());itemIndex->setValue(e->items.size());updating=false;
            record(before);refreshFields();}
    });
    QObject::connect(removeText,&QPushButton::clicked,&dialog,[&] {
        const State before=currentState();
        if(auto* e=selected();e&&e->items.size()>1) {e->items.removeAt(itemIndex->value()-1);
            updating=true;itemIndex->setMaximum(e->items.size());itemIndex->setValue(1);updating=false;record(before);refreshFields();}
    });
    auto persist=[&](QObject* changed) {
        if(updating)return;
        // Take the implicitly-shared snapshot before obtaining mutable pointers.
        // selected() then detaches working, keeping all edits out of history.
        const State before=currentState();
        auto* e=selected();if(!e)return;
        const QPoint wanted(x->value(),y->value());
        // Coordinate edits translate the complete selection/group, just like dragging.
        SelectionGeometry::move(working,preview->selection(),wanted-e->rect.topLeft());
        e->rect.setSize(QSize(w->value(),h->value()));e->mode=mode->currentData().toString();
        e->allLayout=allLayout->currentData().toString();e->columns=columns->value();e->autoSize=autoSize->isChecked();
        e->intervalSeconds=interval->value();e->marquee=marquee->isChecked();e->cycleAfterScroll=cycleAfterScroll->isChecked();
        if(changed==speed)e->scrollPixelsPerSecond=speed->value();
        if(auto* t=selectedText()) {
            t->type=kind->currentData().toString();t->format=format->text();
            // Unavailable fonts and invalid countdown dates may be represented
            // by a fallback in the form. Moving a rectangle must preserve them.
            if(changed==font)t->font=font->currentFont().family();
            t->pointSize=fontSize->value();t->bold=bold->isChecked();
            if(changed==target)t->target=target->dateTime();
            t->vertical=vertical->isChecked();t->abbreviate=abbreviate->isChecked();t->rotateDuty=rotateDuty->isChecked();
            t->weatherApiKey=weatherKey->text();t->weatherAdcode=weatherCode->text();t->weatherCity=weatherCity->text();
            t->weatherAddress=weatherAddress->text();t->weatherForecast=weatherForecast->isChecked();
            t->sourceUrl=sourceUrl->text();t->sourceFile=sourceFile->text();
            if(changed==kind&&!e->autoSize) {
                // Text elements start as short strips; visual widgets need a
                // useful initial viewport when selected from the type menu.
                QSize minimum;
                if(t->type=="analogClock")minimum=QSize(120,140);
                else if(t->type=="weatherBlock")minimum=QSize(200,70);
                else if(t->type=="progressBar")minimum=QSize(180,42);
                if(minimum.isValid())e->rect.setSize(e->rect.size().expandedTo(minimum));
            }
        }
        record(before);preview->update();
        for(int i=0;i<working.size();++i)if(list->item(i))list->item(i)->setText(labelFor(i));
    };
    for(auto* control:{x,y,w,h,interval,fontSize,columns})
        QObject::connect(control,QOverload<int>::of(&QSpinBox::valueChanged),&dialog,[&,control](int){persist(control);});
    QObject::connect(speed,QOverload<double>::of(&QDoubleSpinBox::valueChanged),&dialog,[&](double){persist(speed);});
    for(auto* combo:{kind,mode,allLayout})QObject::connect(combo,QOverload<int>::of(&QComboBox::currentIndexChanged),&dialog,[&,combo](int){if(updating)return;persist(combo);refreshFields();});
    for(auto* field:{format,weatherKey,weatherCode,weatherCity,weatherAddress,sourceUrl,sourceFile})
        QObject::connect(field,&QLineEdit::textChanged,&dialog,[&,field](const QString&){persist(field);});
    QObject::connect(font,&QFontComboBox::currentFontChanged,&dialog,[&](const QFont&){persist(font);});
    for(auto* check:{marquee,bold,vertical,abbreviate,cycleAfterScroll,rotateDuty,weatherForecast,autoSize})
        QObject::connect(check,&QCheckBox::toggled,&dialog,[&,check](bool){if(updating)return;persist(check);refreshFields();});
    QObject::connect(target,&QDateTimeEdit::dateTimeChanged,&dialog,[&](const QDateTime&){persist(target);});
    QObject::connect(color,&QPushButton::clicked,&dialog,[&] {
        if(auto* t=selectedText()) {
            const QColor picked=QColorDialog::getColor(t->color,&dialog,tt::translate("元素文字颜色"),QColorDialog::ShowAlphaChannel);
            if(picked.isValid()){const State before=currentState();selectedText()->color=picked;record(before);refreshFields();}
        }
    });
    QObject::connect(browse,&QPushButton::clicked,&dialog,[&] {
        const QString path=QFileDialog::getOpenFileName(&dialog,tt::translate("选择内容文本文件"),sourceFile->text(),
            tt::translate("内容文件 (*.txt *.md *.json *.rss *.xml);;所有文件 (*)"));
        if(!path.isEmpty())sourceFile->setText(path);
    });
    QObject::connect(buttons,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);
    QObject::connect(buttons,&QDialogButtonBox::accepted,&dialog,&QDialog::accept);
    if(!working.isEmpty())preview->setSelection({0});refreshList();
    history<<currentState();refreshActions();
    // Adding the initial element is undoable, and remains confined to the working copy.
    if(working.isEmpty())add->click();
    if(dialog.exec()!=QDialog::Accepted)return false;
    output=working;return true;
}
}
