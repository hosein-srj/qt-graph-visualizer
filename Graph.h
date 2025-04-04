#ifndef GRAPH_H
#define GRAPH_H

#include <QtWidgets>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsEllipseItem>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFileDialog>

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

class EdgeItem;
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
    QList<EdgeItem*> connectedEdges;

    void addEdge(EdgeItem* edge) {
        connectedEdges.append(edge);
    }

    void removeEdge(EdgeItem* edge) {
        connectedEdges.removeAll(edge);
    }
    QPointF scene_Pos;

signals:
    void positionChanged();
};

class EdgeItem : public QObject, public QGraphicsItem {
    Q_OBJECT
public:
    EdgeItem(NodeItem* startNode, NodeItem* endNode)
        : start(startNode), end(endNode) {
        auto info = staticInformation::instance();
        pen = QPen(info->edgeColor, 2);
        arrowSize = 10;

        connect(start, &NodeItem::positionChanged, this, &EdgeItem::updatePosition);
        connect(end, &NodeItem::positionChanged, this, &EdgeItem::updatePosition);

        start->addEdge(this);
        end->addEdge(this);

        updatePosition();
    }
    ~EdgeItem() {
        if (start) start->removeEdge(this);
        if (end) end->removeEdge(this);
    }

    QRectF boundingRect() const override {
        qreal extra = (pen.width() + arrowSize) / 2.0;
        return QRectF(line.p1(), QSizeF(line.p2().x() - line.p1().x(),
                                        line.p2().y() - line.p1().y()))
            .normalized()
            .adjusted(-extra, -extra, extra, extra);
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override {
        if (!start || !end)
            return;

        painter->setPen(pen);
        painter->drawLine(line);

        painter->setBrush(pen.color());
        painter->drawPolygon(arrowHead);
    }

public slots:
    void updatePosition() {
        auto info = staticInformation::instance();
        int nodeR = info->nodeR / 2;

        QPointF p1 = start->scene_Pos + QPointF(nodeR, nodeR);
        QPointF p2 = end->scene_Pos + QPointF(nodeR, nodeR);

        line = QLineF(p1, p2);  // Full line from start to end

        // Calculate point at 90% of the line (for arrowhead position)
        QPointF arrowTip = p1 + (p2 - p1) * 0.66;

        double angle = std::atan2(-line.dy(), line.dx());

        QPointF arrowP1 = arrowTip - QPointF(std::cos(angle + M_PI / 6) * arrowSize,
                                             -std::sin(angle + M_PI / 6) * arrowSize);
        QPointF arrowP2 = arrowTip - QPointF(std::cos(angle - M_PI / 6) * arrowSize,
                                             -std::sin(angle - M_PI / 6) * arrowSize);

        arrowHead.clear();
        arrowHead << arrowTip << arrowP1 << arrowP2;

        update();
    }
public:
    NodeItem* start;
    NodeItem* end;
private:
    QLineF line;
    QPolygonF arrowHead;
    QPen pen;
    qreal arrowSize;
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
    void exportGraph();
    void importGraph();
private:
    GraphScene *scene;
    QGraphicsView *view;
    StateMouse* stateMouse;
};






#endif // GRAPH_H
