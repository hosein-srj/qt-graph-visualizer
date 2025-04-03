#ifndef GRAPH_H
#define GRAPH_H

#include <QtWidgets>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsEllipseItem>

enum StateMouse{
    Insert_State,
    Remove_State,
    Connect_State,
    Drag_State
};

class staticInformation{
public:
    static staticInformation* instance(){
        static staticInformation* ins = nullptr;
        if(ins == nullptr){
            ins = new staticInformation();
        }
        return ins;
    }
    int nodeR;
    QColor nodeColor;
    QColor edgeColor;
private:
    staticInformation(){
        nodeR = 30;
        nodeColor = QColor(Qt::red);
        edgeColor = QColor(Qt::blue);
    }

};

class NodeItem : public QObject, public QGraphicsEllipseItem {
    Q_OBJECT
public:
    NodeItem(qreal x, qreal y, qreal w, qreal h) : QGraphicsEllipseItem(x, y, w, h) {
        auto info = staticInformation::instance();
        setBrush(info->nodeColor);
        scene_Pos = QPointF(this->rect().x(), this->rect().y());
        //qDebug() << ;

        connect(this, &NodeItem::positionChanged, this, [=](){
            //scene_Pos = QPointF(this->rect().x(), this->rect().y());
            //scene_Pos = scenePos();

            QRectF sceneBoundingRect = this->mapRectToScene(this->rect());
            //qDebug() << scene_Pos << sceneBoundingRect;

            scene_Pos = QPointF(sceneBoundingRect.x(), sceneBoundingRect.y());
        });
    }

    QPointF scene_Pos;

signals:
    void positionChanged();
};



class EdgeItem : public QObject, public QGraphicsLineItem {
    Q_OBJECT
public:
    EdgeItem(NodeItem* startNode, NodeItem* endNode) : start(startNode), end(endNode) {
        auto info = staticInformation::instance();
        int nodeR = info->nodeR / 2;
        setLine(QLineF(start->scene_Pos + QPointF(nodeR, nodeR), end->scene_Pos + QPointF(nodeR, nodeR)));
        setPen(QPen(info->edgeColor, 2));

        // Connect the positionChanged signals of both nodes
        connect(start, &NodeItem::positionChanged, this, &EdgeItem::updatePosition);
        connect(end, &NodeItem::positionChanged, this, &EdgeItem::updatePosition);
    }

public slots:
    void updatePosition() {
        //qDebug() << "47";
        auto info = staticInformation::instance();
        int nodeR = info->nodeR / 2;
        setLine(QLineF(start->scene_Pos + QPointF(nodeR, nodeR), end->scene_Pos + QPointF(nodeR, nodeR)));
    }

private:
    NodeItem* start;
    NodeItem* end;
};

class GraphScene : public QGraphicsScene {
    Q_OBJECT
public:
    GraphScene(StateMouse* state, QObject *parent = nullptr);

    void setNodesMoveAble(bool isMoveAble);
protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    StateMouse* stateMouse;
    QGraphicsLineItem* tempEdge;
    NodeItem* startNode;
};

class Graph : public QWidget {
    Q_OBJECT
public:
    Graph(QWidget *parent = nullptr);

private:
    GraphScene *scene;
    QGraphicsView *view;
    StateMouse* stateMouse;
};






#endif // GRAPH_H
