#pragma once
#include "ElementLayout.h"
#include <QMap>
#include <QSet>
#include <cmath>

namespace tt {
// Geometry shared by the interactive editor and its regressions. Every move
// uses one delta, so snapping never changes spacing inside a selection/group.
class SelectionGeometry {
public:
    enum class Alignment { Left, HorizontalCenter, Right, Top, VerticalCenter, Bottom };
    static QSet<int> expanded(const QList<ElementDefinition>& elements,QSet<int> selected) {
        QSet<QString> groups;
        for(int index:selected)
            if(index>=0&&index<elements.size()&&!elements[index].groupId.isEmpty())
                groups.insert(elements[index].groupId);
        QSet<int> result;
        for(int i=0;i<elements.size();++i)
            if(selected.contains(i)||(!elements[i].groupId.isEmpty()&&groups.contains(elements[i].groupId)))
                result.insert(i);
        return result;
    }
    static QRect bounds(const QList<ElementDefinition>& elements,const QSet<int>& selected) {
        QRect result;
        for(int index:selected)
            if(index>=0&&index<elements.size())result=result.united(elements[index].rect);
        return result;
    }
    static QSet<int> inRectangle(const QList<ElementDefinition>& elements,const QRect& area) {
        QSet<int> result;
        for(int i=0;i<elements.size();++i)
            if(area.intersects(elements[i].rect))result.insert(i);
        return expanded(elements,result);
    }
    static QPoint snappedDelta(const QList<ElementDefinition>& elements,const QSet<int>& selected,
                               QPoint delta,int gridStep,bool snapEdges,int tolerance=6,
                               const QRect& canvas=QRect()) {
        const QRect original=bounds(elements,selected);
        if(original.isEmpty())return delta;
        if(gridStep>0) {
            const QPoint wanted=original.topLeft()+delta;
            delta=QPoint(qRound(qreal(wanted.x())/gridStep)*gridStep,
                         qRound(qreal(wanted.y())/gridStep)*gridStep)-original.topLeft();
        }
        if(!snapEdges)return delta;
        const QRect moving=original.translated(delta);
        int bestX=tolerance+1,bestY=tolerance+1,adjustX=0,adjustY=0;
        auto consider=[&](const QRect& target) {
            const int sourcesX[]={moving.left(),moving.center().x(),moving.right()+1};
            const int sourcesY[]={moving.top(),moving.center().y(),moving.bottom()+1};
            const int targetsX[]={target.left(),target.center().x(),target.right()+1};
            const int targetsY[]={target.top(),target.center().y(),target.bottom()+1};
            for(int source:sourcesX)for(int edge:targetsX) {
                const int correction=edge-source;
                if(std::abs(correction)<bestX){bestX=std::abs(correction);adjustX=correction;}
            }
            for(int source:sourcesY)for(int edge:targetsY) {
                const int correction=edge-source;
                if(std::abs(correction)<bestY){bestY=std::abs(correction);adjustY=correction;}
            }
        };
        for(int i=0;i<elements.size();++i)if(!selected.contains(i))consider(elements[i].rect);
        if(!canvas.isEmpty())consider(canvas);
        return delta+QPoint(bestX<=tolerance?adjustX:0,bestY<=tolerance?adjustY:0);
    }
    static void move(QList<ElementDefinition>& elements,const QSet<int>& selected,QPoint delta) {
        for(int index:selected)
            if(index>=0&&index<elements.size())elements[index].rect.translate(delta);
    }
    static void align(QList<ElementDefinition>& elements,const QSet<int>& selection,Alignment alignment) {
        const QSet<int> selected=expanded(elements,selection);
        const QRect target=bounds(elements,selected);
        // Persistent groups are alignment units, retaining their internal offsets.
        QMap<QString,QSet<int>> units;
        for(int i:selected)
            units[elements[i].groupId.isEmpty()?QString("element:%1").arg(i):
                  QString("group:")+elements[i].groupId].insert(i);
        for(const QSet<int>& unit:units) {
            const QRect source=bounds(elements,unit);
            QPoint delta;
            switch(alignment) {
            case Alignment::Left:delta.setX(target.left()-source.left());break;
            case Alignment::HorizontalCenter:delta.setX(target.center().x()-source.center().x());break;
            case Alignment::Right:delta.setX(target.right()-source.right());break;
            case Alignment::Top:delta.setY(target.top()-source.top());break;
            case Alignment::VerticalCenter:delta.setY(target.center().y()-source.center().y());break;
            case Alignment::Bottom:delta.setY(target.bottom()-source.bottom());break;
            }
            move(elements,unit,delta);
        }
    }
};
}
