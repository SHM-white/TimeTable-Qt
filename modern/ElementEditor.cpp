#include "ElementEditor.h"
#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QDateTimeEdit>
#include <QElapsedTimer>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFontComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QSplitter>
#include <QVBoxLayout>
#include <QTimer>
#include <algorithm>
#include <functional>
#include "QtCompat.h"
namespace tt {
namespace {
const QStringList kinds={"clock","current","next","nextTime","countdown","duty","note","today","progress","week"};
const QStringList displayNames={QStringLiteral("时间"),QStringLiteral("当前课程"),QStringLiteral("下一节课"),
    QStringLiteral("距离下一节"),QStringLiteral("倒计日"),QStringLiteral("每日信息"),
    QStringLiteral("固定文本"),QStringLiteral("今日课表"),QStringLiteral("上课进度"),QStringLiteral("教学周")};
class Preview final:public QWidget {
public:
    Preview(QList<ElementDefinition>& elements,const ScheduleStore& store,QWidget* parent=nullptr)
        :QWidget(parent),elements_(elements),store_(store) {
        setMinimumSize(460,240);
        setMouseTracking(true);
        auto* timer=new QTimer(this);
        QObject::connect(timer,&QTimer::timeout,this,QOverload<>::of(&QWidget::update));
        timer->start(60);
        clock_.start();
    }
    void setActive(int index) {active_=index;update();}
    void setCanvasSize(QSize size) {canvas_=size;update();}
    void setDark(bool dark) {dark_=dark;update();}
    void onMoved(std::function<void()> callback) {onMoved_=std::move(callback);}
protected:
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
        p.setPen(QPen(QColor("#ACB3C5"),1));
        p.drawRect(viewport);
        if(viewport.width()<=0||viewport.height()<=0)return;
        p.save();p.setClipRect(viewport);
        p.translate(viewport.topLeft());
        p.scale(double(viewport.width())/canvas_.width(),double(viewport.height())/canvas_.height());
        ElementLayout::paint(p,QRect(QPoint(0,0),canvas_),elements_,store_,
                             QDateTime::currentDateTime(),clock_.elapsed(),dark_);
        for(int i=0;i<elements_.size();++i) {
            p.setPen(QPen(i==active_?QColor("#625DE1"):QColor("#93A1C8"),i==active_?2:1,Qt::DashLine));
            p.setBrush(Qt::NoBrush);p.drawRect(elements_[i].rect);
        }
        p.restore();
    }
    void mousePressEvent(QMouseEvent* e) override {
        if(e->button()!=Qt::LeftButton)return;
        const QPoint point=toModel(mouseLocalPoint(e).toPoint());
        for(int i=elements_.size()-1;i>=0;--i)if(elements_[i].rect.contains(point)) {
            active_=i;
            down_=point;
            original_=elements_[i].rect.topLeft();
            dragging_=true;
            update();break;
        }
    }
    void mouseMoveEvent(QMouseEvent* e) override {
        if(!dragging_||active_<0||active_>=elements_.size())return;
        const QPoint delta=toModel(mouseLocalPoint(e).toPoint())-down_;
        elements_[active_].rect.moveTopLeft(original_+delta);
        if(onMoved_)onMoved_();
        update();
    }
    void mouseReleaseEvent(QMouseEvent*) override {dragging_=false;}
private:
    QList<ElementDefinition>& elements_;
    const ScheduleStore& store_;
    QElapsedTimer clock_;
    QSize canvas_{420,180};
    int active_=-1;
    bool dark_=false,dragging_=false;
    QPoint down_,original_;
    std::function<void()> onMoved_;
};
} // namespace

bool ElementEditor::edit(QWidget* parent,QList<ElementDefinition>& output,
                         const ScheduleStore& schedule,QSize size,bool dark) {
    QList<ElementDefinition> working=output;  // cancel does not mutate live windows
    QDialog dialog(parent);
    dialog.setWindowTitle(QStringLiteral("信息元素布局编辑器"));
    dialog.resize(1040,650);dialog.setMinimumSize(850,530);
    auto* root=new QVBoxLayout(&dialog);
    auto* info=new QLabel(QStringLiteral("拖动虚线框调整位置；在右侧编辑元素属性。多个元素可以独立放置，支持单项、轮播和同时显示。"));
    info->setWordWrap(true);root->addWidget(info);
    auto* split=new QSplitter(Qt::Horizontal,&dialog);
    auto* left=new QWidget;
    auto* leftCol=new QVBoxLayout(left);
    auto* list=new QListWidget;
    list->setMaximumWidth(270);
    auto* actions=new QHBoxLayout;
    auto* add=new QPushButton(QStringLiteral("新增元素"));
    auto* remove=new QPushButton(QStringLiteral("删除"));
    actions->addWidget(add);actions->addWidget(remove);
    leftCol->addWidget(new QLabel(QStringLiteral("显示元素")));
    leftCol->addWidget(list,1);leftCol->addLayout(actions);
    split->addWidget(left);

    auto* center=new QWidget;
    auto* centerCol=new QVBoxLayout(center);
    auto* preview=new Preview(working,schedule);
    preview->setCanvasSize(QSize(std::clamp(size.width(),240,1200),std::clamp(size.height(),100,700)));
    preview->setDark(dark);
    centerCol->addWidget(new QLabel(QStringLiteral("桌面卡片实时预览 · 鼠标直接拖动元素")),0);
    centerCol->addWidget(preview,1);
    split->addWidget(center);

    auto* right=new QWidget;
    auto* form=new QFormLayout(right);
    auto* kind=new QComboBox;
    for(int i=0;i<kinds.size();++i)kind->addItem(displayNames[i],kinds[i]);
    auto* mode=new QComboBox;
    mode->addItem(QStringLiteral("单一元素"),"single");
    mode->addItem(QStringLiteral("按时间轮播"),"cycle");
    mode->addItem(QStringLiteral("同时纵向显示"),"all");
    auto* itemIndex=new QSpinBox;itemIndex->setRange(1,1);
    auto* addText=new QPushButton(QStringLiteral("增加轮播条目"));
    auto* removeText=new QPushButton(QStringLiteral("移除条目"));
    auto* format=new QLineEdit;
    format->setPlaceholderText(QStringLiteral("支持 %H:%M:%S 或「当前：{value}」"));
    auto* font=new QFontComboBox;
    auto* fontSize=new QSpinBox;fontSize->setRange(7,64);
    auto* bold=new QCheckBox(QStringLiteral("粗体"));
    auto* color=new QPushButton(QStringLiteral("设置文字颜色"));
    auto* x=new QSpinBox;auto* y=new QSpinBox;
    for(auto* v:{x,y})v->setRange(-2048,4096);
    auto* w=new QSpinBox;auto* h=new QSpinBox;
    for(auto* v:{w,h})v->setRange(16,3000);
    auto* interval=new QSpinBox;interval->setRange(1,600);interval->setSuffix(QStringLiteral(" 秒"));
    auto* marquee=new QCheckBox(QStringLiteral("超宽文字滚动"));
    auto* target=new QDateTimeEdit(QDateTime::currentDateTime().addDays(30));
    target->setDisplayFormat("yyyy-MM-dd HH:mm:ss");target->setCalendarPopup(true);
    form->addRow(QStringLiteral("内容类型"),kind);
    form->addRow(QStringLiteral("组合模式"),mode);
    form->addRow(QStringLiteral("条目编号"),itemIndex);
    form->addRow(addText);form->addRow(removeText);
    form->addRow(QStringLiteral("文本 / 格式"),format);
    form->addRow(QStringLiteral("字体"),font);
    form->addRow(QStringLiteral("字号"),fontSize);
    form->addRow(bold);form->addRow(color);
    form->addRow(QStringLiteral("X"),x);form->addRow(QStringLiteral("Y"),y);
    form->addRow(QStringLiteral("宽度"),w);form->addRow(QStringLiteral("高度"),h);
    form->addRow(QStringLiteral("轮播间隔"),interval);
    form->addRow(marquee);
    form->addRow(QStringLiteral("倒计目标"),target);
    split->addWidget(right);
    split->setStretchFactor(0,0);split->setStretchFactor(1,2);split->setStretchFactor(2,1);
    root->addWidget(split,1);
    auto* buttons=new QDialogButtonBox(QDialogButtonBox::Save|QDialogButtonBox::Cancel);
    buttons->button(QDialogButtonBox::Save)->setText(QStringLiteral("保存布局"));
    root->addWidget(buttons);

    bool updating=false;
    auto selected=[&]()->ElementDefinition* {
        int n=list->currentRow();return n>=0&&n<working.size()?&working[n]:nullptr;
    };
    auto selectedText=[&]()->TextElement* {
        ElementDefinition* e=selected();
        if(!e)return nullptr;
        const int n=itemIndex->value()-1;
        return n>=0&&n<e->items.size()?&e->items[n]:nullptr;
    };
    std::function<void()> refreshList;
    std::function<void()> refreshFields;
    refreshList=[&] {
        int previous=list->currentRow();
        list->clear();
        for(int i=0;i<working.size();++i)
            list->addItem(QStringLiteral("%1. %2  (%3,%4)").arg(i+1)
                .arg(working[i].items.isEmpty()?QStringLiteral("空元素"):working[i].items.first().type)
                .arg(working[i].rect.x()).arg(working[i].rect.y()));
        if(!working.isEmpty())list->setCurrentRow(std::clamp(previous,0,int(working.size())-1));
        preview->update();
    };
    refreshFields=[&] {
        updating=true;
        if(ElementDefinition* e=selected()) {
            x->setValue(e->rect.x());y->setValue(e->rect.y());
            w->setValue(e->rect.width());h->setValue(e->rect.height());
            mode->setCurrentIndex(std::max(0,mode->findData(e->mode)));
            interval->setValue(e->intervalSeconds);marquee->setChecked(e->marquee);
            itemIndex->setMaximum(std::max(1,int(e->items.size())));
            if(itemIndex->value()>e->items.size())itemIndex->setValue(1);
            if(TextElement* t=selectedText()) {
                kind->setCurrentIndex(std::max(0,kind->findData(t->type)));
                format->setText(t->format);
                font->setCurrentFont(QFont(t->font));
                fontSize->setValue(t->pointSize);bold->setChecked(t->bold);
                color->setStyleSheet("background-color:"+t->color.name()+";color:"+(
                    t->color.lightness()<128?"white":"black")+";");
                target->setDateTime(t->target.isValid()?t->target:QDateTime::currentDateTime().addDays(30));
            }
        }
        preview->setActive(list->currentRow());
        updating=false;
    };
    QObject::connect(list,&QListWidget::currentRowChanged,&dialog,[&](int){refreshFields();});
    QObject::connect(itemIndex,QOverload<int>::of(&QSpinBox::valueChanged),&dialog,[&](int){if(!updating)refreshFields();});
    QObject::connect(add,&QPushButton::clicked,&dialog,[&]{
        ElementDefinition e;
        e.rect=QRect(10+int(working.size())*12,10+int(working.size())*12,170,30);
        TextElement t;t.type="clock";t.format="%H:%M:%S";e.items<<t;
        working<<e;refreshList();list->setCurrentRow(working.size()-1);
    });
    QObject::connect(remove,&QPushButton::clicked,&dialog,[&]{
        int n=list->currentRow();if(n<0||n>=working.size())return;
        working.removeAt(n);refreshList();
    });
    QObject::connect(addText,&QPushButton::clicked,&dialog,[&]{
        if(auto* e=selected()) {
            e->items<<TextElement{};
            itemIndex->setMaximum(e->items.size());
            itemIndex->setValue(e->items.size());refreshFields();
        }
    });
    QObject::connect(removeText,&QPushButton::clicked,&dialog,[&]{
        if(auto* e=selected()) {
            if(e->items.size()<=1)return;
            e->items.removeAt(itemIndex->value()-1);
            itemIndex->setMaximum(e->items.size());itemIndex->setValue(1);refreshFields();
        }
    });
    auto persist=[&] {
        if(updating)return;
        auto* e=selected();if(!e)return;
        e->rect=QRect(x->value(),y->value(),w->value(),h->value());
        e->mode=mode->currentData().toString();
        e->intervalSeconds=interval->value();e->marquee=marquee->isChecked();
        if(TextElement* t=selectedText()) {
            t->type=kind->currentData().toString();
            t->format=format->text();t->font=font->currentFont().family();
            t->pointSize=fontSize->value();t->bold=bold->isChecked();t->target=target->dateTime();
        }
        preview->update();
    };
    for(auto* control:{x,y,w,h,interval,fontSize})connect(control,QOverload<int>::of(&QSpinBox::valueChanged),&dialog,[&](int){persist();});
    QObject::connect(kind,QOverload<int>::of(&QComboBox::currentIndexChanged),&dialog,[&](int){persist();});
    QObject::connect(mode,QOverload<int>::of(&QComboBox::currentIndexChanged),&dialog,[&](int){persist();});
    QObject::connect(format,&QLineEdit::textChanged,&dialog,[&](const QString&){persist();});
    QObject::connect(font,&QFontComboBox::currentFontChanged,&dialog,[&](const QFont&){persist();});
    QObject::connect(marquee,&QCheckBox::toggled,&dialog,[&](bool){persist();});
    QObject::connect(bold,&QCheckBox::toggled,&dialog,[&](bool){persist();});
    QObject::connect(target,&QDateTimeEdit::dateTimeChanged,&dialog,[&](const QDateTime&){persist();});
    QObject::connect(color,&QPushButton::clicked,&dialog,[&]{
        if(auto* text=selectedText()) {
            const QColor picked=QColorDialog::getColor(text->color,&dialog,
                QStringLiteral("元素文字颜色"),QColorDialog::ShowAlphaChannel);
            if(picked.isValid()){text->color=picked;refreshFields();preview->update();}
        }
    });
    preview->onMoved([&] {
        updating=true;
        if(auto* e=selected()){x->setValue(e->rect.x());y->setValue(e->rect.y());}
        updating=false;
    });
    QObject::connect(buttons,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);
    QObject::connect(buttons,&QDialogButtonBox::accepted,&dialog,&QDialog::accept);
    refreshList();
    if(working.isEmpty())add->click();
    if(dialog.exec()!=QDialog::Accepted)return false;
    output=working;
    return true;
}
}
