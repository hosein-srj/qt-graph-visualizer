#include "Graph.h"


GraphScene::GraphScene(StateMouse *state, QObject *parent) : QGraphicsScene(parent), stateMouse(state), tempEdge(nullptr), startNode(nullptr) {

}

void GraphScene::setNodesMoveAble(bool isMoveAble)
{
    auto itemsAtScene = items();
    for (QGraphicsItem *item : itemsAtScene) {
        auto node = dynamic_cast<NodeItem*>(item);
        if (node) {
            if(isMoveAble)
            {
                node->setFlag(QGraphicsItem::ItemIsMovable, true);
                node->setFlag(QGraphicsItem::ItemIsSelectable, true);
            }
            else
            {
                node->setFlag(QGraphicsItem::ItemIsMovable, false);
                node->setFlag(QGraphicsItem::ItemIsSelectable, false);
            }
        }
    }
}

void GraphScene::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    if (*stateMouse == Insert_State) {
        QGraphicsItem *clickedItem = itemAt(event->scenePos(), QTransform());
        if (!clickedItem && event->button() == Qt::LeftButton) {
            auto info = staticInformation::instance();
            int nodeR = info->nodeR / 2;
            auto node = new NodeItem(event->scenePos().x() - nodeR, event->scenePos().y() - nodeR, nodeR * 2, nodeR * 2);

            addItem(node);
        }
    } else if (*stateMouse == Remove_State) {
        QGraphicsItem *clickedItem = itemAt(event->scenePos(), QTransform());
        if (clickedItem && event->button() == Qt::LeftButton) {
            removeItem(clickedItem);
            delete clickedItem;
        }
    } else if (*stateMouse == Connect_State) {
        QGraphicsItem *clickedItem = itemAt(event->scenePos(), QTransform());
        if (clickedItem && event->button() == Qt::LeftButton) {
            NodeItem* node = dynamic_cast<NodeItem*>(clickedItem);
            if (node) {
                startNode = node;
                auto info = staticInformation::instance();
                int nodeR = info->nodeR / 2;

                tempEdge = new QGraphicsLineItem(QLineF(node->scene_Pos + QPointF(nodeR, nodeR), event->scenePos()));
                tempEdge->setPen(QPen(info->edgeColor, 2, Qt::DashLine));
                addItem(tempEdge);
            }
        }
    }
    QGraphicsScene::mousePressEvent(event);
}

void GraphScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    // Update the edge while dragging
    if (tempEdge) {
        auto info = staticInformation::instance();
        int nodeR = info->nodeR / 2;
        tempEdge->setLine(QLineF(startNode->scene_Pos + QPointF(nodeR, nodeR), event->scenePos()));
    }

    // Check if a node is moved
    if(*stateMouse == Drag_State){
        QList<QGraphicsItem *> itemsAtScene = items(event->scenePos());
        for (QGraphicsItem *item : itemsAtScene) {
            NodeItem* node = dynamic_cast<NodeItem*>(item);
            if (node) {
                emit node->positionChanged();
            }
        }
    }


    QGraphicsScene::mouseMoveEvent(event);
}

void GraphScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    if (tempEdge && startNode) {
        // QGraphicsItem *clickedItem = itemAt(event->scenePos(), QTransform());
        // NodeItem* endNode = dynamic_cast<NodeItem*>(clickedItem);
        QList<QGraphicsItem *> itemsUnderCursor = items(event->scenePos());
        NodeItem* endNode = nullptr;

        // Find a NodeItem under the cursor
        for (QGraphicsItem *item : itemsUnderCursor) {
            endNode = dynamic_cast<NodeItem*>(item);
            if (endNode) {
                break;
            }
        }

        if (endNode && endNode != startNode) {
            //qDebug() << "92";
            EdgeItem* edge = new EdgeItem(startNode, endNode);
            addItem(edge);
        }
        // else{
        //     qDebug() << "97";
        // }
        removeItem(tempEdge);
        delete tempEdge;
        tempEdge = nullptr;
        startNode = nullptr;
    }
    QGraphicsScene::mouseReleaseEvent(event);
}

Graph::Graph(QWidget *parent) : QWidget(parent) {
    QVBoxLayout* laymain = new QVBoxLayout();
    laymain->setContentsMargins(0,0,0,0);

    QHBoxLayout* layTop = new QHBoxLayout();
    layTop->setContentsMargins(0,0,0,0);
    layTop->setSpacing(10);

    QPushButton* btnInsert = new QPushButton("Insert");
    QPushButton* btnRemove = new QPushButton("Remove");
    QPushButton* btnConnect = new QPushButton("Connect");
    QPushButton* btnDrag = new QPushButton("Drag");

    connect(btnInsert, &QPushButton::clicked, this, [=](){
        *stateMouse = Insert_State;
        scene->setNodesMoveAble(false);
    });
    connect(btnRemove, &QPushButton::clicked, this, [=](){
        *stateMouse = Remove_State;
        scene->setNodesMoveAble(false);
    });
    connect(btnConnect, &QPushButton::clicked, this, [=](){
        *stateMouse = Connect_State;
        scene->setNodesMoveAble(false);
    });
    connect(btnDrag, &QPushButton::clicked, this, [=](){
        *stateMouse = Drag_State;
        scene->setNodesMoveAble(true);
    });

    stateMouse = new StateMouse();
    *stateMouse = Insert_State;

    layTop->addWidget(btnInsert);
    layTop->addWidget(btnRemove);
    layTop->addWidget(btnDrag);
    layTop->addWidget(btnConnect);

    scene = new GraphScene(stateMouse, this);
    view = new QGraphicsView(scene, this);

    laymain->addLayout(layTop);
    laymain->addWidget(view);
    setLayout(laymain);
    scene->setSceneRect(0, 0, 800, 600);
}
