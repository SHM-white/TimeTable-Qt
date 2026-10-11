#include "ElementEditor.h"
#include "ElementSelection.h"
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QDebug>
#include <QLineEdit>
#include <QListWidget>
#include <QMouseEvent>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QSpinBox>
#include <QTimer>
#include <functional>

using namespace tt;
namespace {
int failures=0;
void check(bool condition,const char* name) {
    if(!condition){qCritical()<<"FAILED:"<<name;++failures;}
}
ElementDefinition element(QRect rect,QString group={}) {
    ElementDefinition e;e.rect=rect;e.groupId=group;
    TextElement t;t.type="note";t.format="Original text";e.items={t};return e;
}
template<class Widget> Widget* control(QDialog* dialog,const char* name) {
    auto* result=dialog->findChild<Widget*>(name);
    check(result!=nullptr,name);return result;
}
bool editWith(QList<ElementDefinition>& layout,const std::function<void(QDialog*)>& interact) {
    ScheduleStore schedule;
    QTimer watchdog;watchdog.setSingleShot(true);
    QObject::connect(&watchdog,&QTimer::timeout,[&] {
        check(false,"editor interaction completes without hanging");
        if(auto* dialog=qobject_cast<QDialog*>(QApplication::activeModalWidget()))dialog->reject();
    });
    watchdog.start(5000);
    QTimer::singleShot(0,[&] {
        auto* dialog=qobject_cast<QDialog*>(QApplication::activeModalWidget());
        check(dialog&&dialog->objectName()=="elementEditorDialog","actual transactional editor opened");
        if(dialog)interact(dialog);
    });
    const bool accepted=ElementEditor::edit(nullptr,layout,schedule,QSize(420,180),false);
    watchdog.stop();return accepted;
}
QPoint previewPoint(QWidget* preview,QPoint model) {
    const QSize scaled=QSize(420,180).scaled(preview->size()-QSize(24,24),Qt::KeepAspectRatio);
    const QPoint origin((preview->width()-scaled.width())/2,(preview->height()-scaled.height())/2);
    return origin+QPoint(model.x()*scaled.width()/420,model.y()*scaled.height()/180);
}
void mouse(QWidget* preview,QEvent::Type type,QPoint model,Qt::KeyboardModifiers modifiers=Qt::NoModifier) {
    const QPoint point=previewPoint(preview,model);
    QMouseEvent event(type,QPointF(point),QPointF(preview->mapToGlobal(point)),
        type==QEvent::MouseMove?Qt::NoButton:Qt::LeftButton,
        type==QEvent::MouseButtonRelease?Qt::NoButton:Qt::LeftButton,modifiers);
    QApplication::sendEvent(preview,&event);
}
}
int main(int argc,char** argv) {
    QApplication app(argc,argv);app.setQuitOnLastWindowClosed(false);
    QList<ElementDefinition> geometry={element(QRect(10,20,40,20),"pair"),
        element(QRect(60,30,20,20),"pair"),element(QRect(100,90,20,20))};
    check(SelectionGeometry::expanded(geometry,{0})==QSet<int>({0,1}),
          "picking one member expands persistent group");
    check(SelectionGeometry::expanded(geometry,{-1,3}).isEmpty(),
          "invalid selection indices never enter geometry operations");
    check(SelectionGeometry::inRectangle(geometry,QRect(9,19,3,3))==QSet<int>({0,1}),
          "rubber band intersection selects the whole group");
    const QPoint gap=geometry[1].rect.topLeft()-geometry[0].rect.topLeft();
    const QSet<int> pair{0,1};
    const QPoint snapped=SelectionGeometry::snappedDelta(geometry,pair,QPoint(4,4),8,false);
    SelectionGeometry::move(geometry,pair,snapped);
    check(geometry[0].rect.topLeft()==QPoint(16,24),"grid snapping uses selection anchor");
    check(geometry[1].rect.topLeft()-geometry[0].rect.topLeft()==gap,
          "one snapped delta preserves all group offsets");
    check(geometry[2].rect==QRect(100,90,20,20),"group movement leaves unrelated elements untouched");
    auto edgeGeometry=QList<ElementDefinition>{element(QRect(10,20,40,20)),
        element(QRect(60,30,20,20)),element(QRect(100,20,20,30))};
    check(SelectionGeometry::snappedDelta(edgeGeometry,pair,QPoint(23,0),0,true)==QPoint(20,0),
          "selection outer edge snaps to another element within tolerance");
    check(SelectionGeometry::snappedDelta(edgeGeometry,pair,QPoint(23,0),0,false)==QPoint(23,0),
          "turning off snapping permits pixel precise placement");
    SelectionGeometry::align(geometry,{0,1,2},SelectionGeometry::Alignment::Left);
    check(geometry[2].rect.left()==geometry[0].rect.left(),"left alignment operates across independent units");
    check(geometry[1].rect.topLeft()-geometry[0].rect.topLeft()==gap,
          "aligning a persistent group preserves its shape");

    QList<ElementDefinition> original={element(QRect(13,19,170,30)),element(QRect(100,70,90,30))};
    const QJsonArray originalJson=ElementLayout::toJson(original);
    auto cancelled=original;
    const bool cancelResult=editWith(cancelled,[&](QDialog* dialog) {
        if(auto* field=control<QLineEdit>(dialog,"editorFormat"))field->setText("Uncommitted edit");
        if(auto* spin=control<QSpinBox>(dialog,"editorX"))spin->setValue(91);
        if(auto* button=control<QPushButton>(dialog,"editorAdd"))button->click();
        if(auto* scroll=control<QScrollArea>(dialog,"editorPropertyScroll")) {
            check(scroll->widgetResizable(),"property form is inside resizable scroll area");
            check(scroll->verticalScrollBar()->maximum()>0,"long property form scrolls instead of expanding dialog");
        }
        dialog->reject();
    });
    check(!cancelResult&&ElementLayout::toJson(cancelled)==originalJson,
          "cancel rolls back content, geometry and additions together");

    auto inspected=original;
    inspected[1].mode="all";inspected[1].allLayout="horizontal";inspected[1].autoSize=true;inspected[1].marquee=false;
    inspected[1].items[0].type="clock";inspected[1].items[0].format="%H:%M:%S";inspected[1].items[0].bold=true;
    const QJsonArray inspectedJson=ElementLayout::toJson(inspected);
    check(editWith(inspected,[&](QDialog* dialog) {
        if(auto* list=control<QListWidget>(dialog,"editorElements")) {list->setCurrentRow(1);list->setCurrentRow(0);}
        if(auto* undo=control<QPushButton>(dialog,"editorUndo"))check(!undo->isEnabled(),"inspecting different fields never creates an edit");
        if(auto* buttons=control<QDialogButtonBox>(dialog,"editorButtons"))buttons->button(QDialogButtonBox::Save)->click();
    }),"inspection only editor session can be saved");
    check(ElementLayout::toJson(inspected)==inspectedJson,
          "changing selection across distinct modes and kinds preserves every model field");

    auto undoLayout=original;
    check(editWith(undoLayout,[&](QDialog* dialog) {
        auto* x=control<QSpinBox>(dialog,"editorX");auto* undo=control<QPushButton>(dialog,"editorUndo");
        auto* redo=control<QPushButton>(dialog,"editorRedo");
        if(x)x->setValue(67);
        if(undo){check(undo->isEnabled(),"geometry edits enable undo");undo->click();}
        if(x)check(x->value()==13,"undo restores previous coordinates in form and model");
        if(redo){check(redo->isEnabled(),"undo enables redo");redo->click();}
        if(x)check(x->value()==67,"redo reapplies the geometry edit");
        if(undo)undo->click();
        if(x)x->setValue(81);
        if(redo)check(!redo->isEnabled(),"editing after undo discards the abandoned redo branch");
        if(undo)undo->click();
        if(auto* buttons=control<QDialogButtonBox>(dialog,"editorButtons"))buttons->button(QDialogButtonBox::Save)->click();
    }),"save accepts editor after history traversal");
    check(ElementLayout::toJson(undoLayout)==originalJson,"undo then save commits the exact original snapshot");

    auto configured=original;
    check(editWith(configured,[&](QDialog* dialog) {
        if(auto* mode=control<QComboBox>(dialog,"editorMode"))mode->setCurrentIndex(mode->findData("cycle"));
        if(auto* cycle=control<QCheckBox>(dialog,"editorCycleAfterScroll"))cycle->setChecked(true);
        if(auto* speed=control<QDoubleSpinBox>(dialog,"editorScrollSpeed"))speed->setValue(45.2);
        if(auto* width=control<QSpinBox>(dialog,"editorWidth"))width->setValue(220);
        auto* add=control<QPushButton>(dialog,"editorAddText");auto* remove=control<QPushButton>(dialog,"editorRemoveText");
        auto* undo=control<QPushButton>(dialog,"editorUndo");auto* redo=control<QPushButton>(dialog,"editorRedo");
        auto* index=control<QSpinBox>(dialog,"editorItemIndex");
        if(add)add->click();
        if(index)check(index->maximum()==2,"adding a cycle entry changes the working model");
        if(undo)undo->click();
        if(index)check(index->maximum()==1,"undo restores item collection before adding");
        if(redo)redo->click();
        if(remove)remove->click();
        if(index)check(index->maximum()==1,"removing an entry changes the working model");
        if(undo)undo->click();
        if(index)check(index->maximum()==2,"undo restores the removed entry");
        if(auto* buttons=control<QDialogButtonBox>(dialog,"editorButtons"))buttons->button(QDialogButtonBox::Save)->click();
    }),"cycle properties and collection history commit");
    check(configured[0].mode=="cycle"&&configured[0].cycleAfterScroll&&qAbs(configured[0].scrollPixelsPerSecond-45.2)<0.01,
          "element mode and scrolling properties reach the working copy");
    check(configured[0].rect.width()==220&&configured[0].items.size()==2,
          "dimension changes and item collection survive snapshot detachment");

    auto dragged=original;
    check(editWith(dragged,[&](QDialog* dialog) {
        auto* preview=control<QWidget>(dialog,"elementPreview");auto* list=control<QListWidget>(dialog,"editorElements");
        if(auto* grid=control<QCheckBox>(dialog,"editorGrid"))grid->setChecked(false);
        if(auto* edges=control<QCheckBox>(dialog,"editorEdgeSnap"))edges->setChecked(false);
        if(preview&&list) {
            mouse(preview,QEvent::MouseButtonPress,QPoint(120,80),Qt::ControlModifier);
            mouse(preview,QEvent::MouseButtonRelease,QPoint(120,80),Qt::ControlModifier);
            check(list->selectedItems().size()==2,"Ctrl click adds a second element to actual preview selection");
            mouse(preview,QEvent::MouseButtonPress,QPoint(30,30));
            mouse(preview,QEvent::MouseMove,QPoint(55,46));
            mouse(preview,QEvent::MouseButtonRelease,QPoint(55,46));
            if(auto* undo=control<QPushButton>(dialog,"editorUndo"))undo->click();
            if(auto* spin=control<QSpinBox>(dialog,"editorX"))check(spin->value()==13,"one undo restores a complete multi element drag");
            if(auto* redo=control<QPushButton>(dialog,"editorRedo"))redo->click();
            // Starting in empty canvas clears selection and rubber-bands both elements.
            mouse(preview,QEvent::MouseButtonPress,QPoint(4,4));
            mouse(preview,QEvent::MouseMove,QPoint(240,125));
            mouse(preview,QEvent::MouseButtonRelease,QPoint(240,125));
            check(list->selectedItems().size()==2,"actual rubber band selects intersecting elements");
        }
        if(auto* buttons=control<QDialogButtonBox>(dialog,"editorButtons"))buttons->button(QDialogButtonBox::Save)->click();
    }),"preview selection and drag commit");
    check(dragged[0].rect.x()>original[0].rect.x()&&dragged[0].rect.y()>original[0].rect.y(),
          "actual preview drag updates saved coordinates");
    check(dragged[1].rect.topLeft()-dragged[0].rect.topLeft()==original[1].rect.topLeft()-original[0].rect.topLeft(),
          "actual multi element drag preserves selection spacing");

    auto grouped=original;
    check(editWith(grouped,[&](QDialog* dialog) {
        auto* list=control<QListWidget>(dialog,"editorElements");
        if(list){list->item(0)->setSelected(true);list->item(1)->setSelected(true);}
        if(auto* button=control<QPushButton>(dialog,"editorGroup")){check(button->isEnabled(),"multi selection enables grouping");button->click();}
        if(auto* spin=control<QSpinBox>(dialog,"editorX"))spin->setValue(33);
        if(auto* buttons=control<QDialogButtonBox>(dialog,"editorButtons"))buttons->button(QDialogButtonBox::Save)->click();
    }),"group and translation can be saved");
    check(!grouped[0].groupId.isEmpty()&&grouped[0].groupId==grouped[1].groupId,
          "group identifiers persist in the committed layout");
    check(grouped[0].rect.x()==33&&grouped[1].rect.x()==120,
          "editing one group member coordinate moves the complete selection by a common delta");
    const auto roundTrip=ElementLayout::fromJson(ElementLayout::toJson(grouped));
    check(roundTrip[0].groupId==grouped[0].groupId&&roundTrip[1].groupId==grouped[1].groupId,
          "group membership survives JSON reload");
    check(editWith(grouped,[&](QDialog* dialog) {
        if(auto* button=control<QPushButton>(dialog,"editorUngroup")){check(button->isEnabled(),"loaded group enables ungroup action");button->click();}
        if(auto* buttons=control<QDialogButtonBox>(dialog,"editorButtons"))buttons->button(QDialogButtonBox::Save)->click();
    }),"ungroup commits");
    check(grouped[0].groupId.isEmpty()&&grouped[1].groupId.isEmpty(),"ungroup clears every member of a selected group");

    auto types=original;
    check(editWith(types,[&](QDialog* dialog) {
        if(auto* kind=control<QComboBox>(dialog,"editorKind")) {
            for(const char* type:{"news","dailyWord","progressBar","analogClock","weatherBlock"})
                check(kind->findData(type)>=0,"newly supported content kind is editable");
            kind->setCurrentIndex(kind->findData("analogClock"));
        }
        if(auto* buttons=control<QDialogButtonBox>(dialog,"editorButtons"))buttons->button(QDialogButtonBox::Save)->click();
    }),"new content kind can be saved");
    check(types.first().items.first().type=="analogClock","new type commits through actual editor controls");
    check(types.first().rect.height()>=140,"selecting analog clock gives it a usable viewport");
    if(!failures)qInfo()<<"Editor selection and transaction regressions passed";
    return failures?1:0;
}
