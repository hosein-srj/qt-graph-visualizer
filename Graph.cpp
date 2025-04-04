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
            NodeItem* node = dynamic_cast<NodeItem*>(clickedItem);
            if (node) {
                // Delete all connected edges
                auto edgesCopy = node->connectedEdges; // Make a copy to avoid modifying during iteration
                for (EdgeItem* edge : edgesCopy) {
                    removeItem(edge);
                    delete edge;
                }
                node->connectedEdges.clear(); // Just to be safe
            }

            removeItem(clickedItem);
            delete clickedItem;
        }
        //QGraphicsItem *clickedItem = itemAt(event->scenePos(), QTransform());
        //if (clickedItem && event->button() == Qt::LeftButton) {
        //    removeItem(clickedItem);
        //    delete clickedItem;
        //}
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

    QGridLayout* layTop = new QGridLayout();
    layTop->setContentsMargins(5,5,5,5);
    layTop->setSpacing(10);

    QPushButton* btnInsert = new QPushButton("Insert");
    QPushButton* btnRemove = new QPushButton("Remove");
    QPushButton* btnConnect = new QPushButton("Connect");
    QPushButton* btnDrag = new QPushButton("Drag");

    QLabel* lblRadius = new QLabel("Node Radius: ");
    QSpinBox* spinRadius = new QSpinBox();
    connect(spinRadius, &QSpinBox::valueChanged, this, [=](int){
        auto info = staticInformation::instance();
        info->nodeR = spinRadius->value();
    });

    QPushButton* btnNodeColor = new QPushButton("Node Color");
    connect(btnNodeColor, &QPushButton::clicked, this, [=](){
        QColor color = QColorDialog::getColor();
        auto info = staticInformation::instance();
        info->nodeColor = color;
        btnNodeColor->setStyleSheet("background-color: " + color.name() + "; " + btnNodeColor->styleSheet());
    });

    QPushButton* btnEdgeColor = new QPushButton("Node Color");
    connect(btnEdgeColor, &QPushButton::clicked, this, [=](){
        QColor color = QColorDialog::getColor();
        auto info = staticInformation::instance();
        info->edgeColor = color;
        btnEdgeColor->setStyleSheet("background-color: " + color.name() + "; " + btnEdgeColor->styleSheet());
    });

    QPushButton* btnExport = new QPushButton("Export");
    QPushButton* btnImport = new QPushButton("Import");
    connect(btnExport, &QPushButton::clicked, this, [=](){
        exportGraph();
    });
    connect(btnImport, &QPushButton::clicked, this, [=](){
        importGraph();
    });

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

    layTop->addWidget(btnInsert, 0, 0);
    layTop->addWidget(btnRemove, 0, 1);
    layTop->addWidget(btnDrag, 0, 2);
    layTop->addWidget(btnConnect, 0, 3);
    layTop->addWidget(btnExport, 0, 4);
    layTop->addWidget(lblRadius, 1, 0);
    layTop->addWidget(spinRadius, 1, 1);
    layTop->addWidget(btnNodeColor, 1, 2);
    layTop->addWidget(btnEdgeColor, 1, 3);
    layTop->addWidget(btnImport, 1, 4);

    scene = new GraphScene(stateMouse, this);
    view = new QGraphicsView(scene, this);

    laymain->addLayout(layTop);
    laymain->addWidget(view);
    setLayout(laymain);
    scene->setSceneRect(0, 0, 800, 600);
}



void Graph::exportGraph() {
    QJsonArray nodeArray;
    QJsonArray edgeArray;

    // Serialize all nodes
    for (QGraphicsItem* item : scene->items()) {
        if (NodeItem* node = dynamic_cast<NodeItem*>(item)) {
            QJsonObject nodeObj;
            nodeObj["x"] = node->scene_Pos.x();
            nodeObj["y"] = node->scene_Pos.y();
            nodeObj["radius"] = staticInformation::instance()->nodeR;
            nodeArray.append(nodeObj);
        }
    }

    // Serialize all edges
    for (QGraphicsItem* item : scene->items()) {
        if (EdgeItem* edge = dynamic_cast<EdgeItem*>(item)) {
            QJsonObject edgeObj;
            QJsonObject startPoint;
            startPoint["x"] = edge->start->scene_Pos.x();
            startPoint["y"] = edge->start->scene_Pos.y();

            QJsonObject endPoint;
            endPoint["x"] = edge->end->scene_Pos.x();
            endPoint["y"] = edge->end->scene_Pos.y();

            edgeObj["startNode"] = startPoint;
            edgeObj["endNode"] = endPoint;
            edgeArray.append(edgeObj);
        }
    }

    // Create JSON document
    QJsonObject graphObj;
    graphObj["nodes"] = nodeArray;
    graphObj["edges"] = edgeArray;

    // Convert to QByteArray for saving to file
    QJsonDocument doc(graphObj);
    QByteArray data = doc.toJson();

    // Save to file
    QString fileName = QFileDialog::getSaveFileName(this, "Save Graph", "", "JSON Files (*.json)");
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(data);
            file.close();
        }
    }
}

void Graph::importGraph() {
    QString fileName = QFileDialog::getOpenFileName(this, "Open Graph", "", "JSON Files (*.json)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) return;

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject graphObj = doc.object();

    // Clear current scene
    scene->clear();

    // Load nodes
    QJsonArray nodeArray = graphObj["nodes"].toArray();
    for (const QJsonValue& value : nodeArray) {
        QJsonObject nodeObj = value.toObject();
        qreal x = nodeObj["x"].toDouble();
        qreal y = nodeObj["y"].toDouble();
        int radius = nodeObj["radius"].toInt();

        auto node = new NodeItem(x, y, radius, radius);
        scene->addItem(node);
    }

    // Load edges
    QJsonArray edgeArray = graphObj["edges"].toArray();
    for (const QJsonValue& value : edgeArray) {
        QJsonObject edgeObj = value.toObject();
        QJsonObject startPointObj = edgeObj["startNode"].toObject();
        QJsonObject endPointObj = edgeObj["endNode"].toObject();

        QPointF startPos(startPointObj["x"].toDouble(), startPointObj["y"].toDouble());
        QPointF endPos(endPointObj["x"].toDouble(), endPointObj["y"].toDouble());
        NodeItem* startNode = nullptr;
        NodeItem* endNode = nullptr;

        for (QGraphicsItem* item : scene->items()) {
            if (NodeItem* node = dynamic_cast<NodeItem*>(item)) {
                if (node->scene_Pos == startPos) {
                    startNode = node;
                }
                if (node->scene_Pos == endPos) {
                    endNode = node;
                }
                //qDebug() << node->scene_Pos << startPos << endPos;
            }
        }
        if (startNode && endNode) {
            EdgeItem* edge = new EdgeItem(startNode, endNode);
            scene->addItem(edge);
        }
    }

    file.close();
}

