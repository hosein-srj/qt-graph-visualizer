#include "Graph.h"


GraphScene::GraphScene(StateMouse *state, QObject *parent) : QGraphicsScene(parent), stateMouse(state), tempEdge(nullptr), startNode(nullptr) {

}

void GraphScene::clearScene()
{
    QList<QGraphicsItem*> allItems = items();
    for (QGraphicsItem* item : allItems) {
        removeItem(item);
        delete item;
    }
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
            QString label = QInputDialog::getText(nullptr, "Node Label", "Enter node label:");
            auto node = new NodeItem(event->scenePos().x() - nodeR, event->scenePos().y() - nodeR, nodeR * 2, nodeR * 2, label);

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
                update();
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
            //auto info = staticInformation::instance();
            double weight = QInputDialog::getDouble(nullptr, "Edge Weight", "Enter edge weight:");
            EdgeItem* edge = new EdgeItem(startNode, endNode, weight);
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

    QPushButton* btnClear = new QPushButton("Clear");
    connect(btnClear, &QPushButton::clicked, this, [=]() {
        scene->clearScene();
    });

    stateMouse = new StateMouse();
    *stateMouse = Insert_State;

    layTop->addWidget(btnInsert, 0, 0);
    layTop->addWidget(btnRemove, 0, 1);
    layTop->addWidget(btnDrag, 0, 2);
    layTop->addWidget(btnConnect, 0, 3);
    layTop->addWidget(btnClear, 0, 4);

    layTop->addWidget(lblRadius, 1, 0);
    layTop->addWidget(spinRadius, 1, 1);
    layTop->addWidget(btnNodeColor, 1, 2);
    layTop->addWidget(btnEdgeColor, 1, 3);
    layTop->addWidget(btnImport, 1, 4);
    layTop->addWidget(btnExport, 1, 5);

    scene = new GraphScene(stateMouse, this);
    view = new QGraphicsView(scene, this);

    laymain->addLayout(layTop);
    laymain->addWidget(view);
    setLayout(laymain);
    scene->setSceneRect(0, 0, 800, 600);
}



void Graph::exportGraph() {
    QJsonArray nodesArray;
    QJsonArray edgesArray;
    auto itemsInScene = scene->items();

    // Collect nodes
    for (QGraphicsItem* item : itemsInScene) {
        NodeItem* node = dynamic_cast<NodeItem*>(item);
        if (node) {
            QJsonObject nodeObj;
            nodeObj["x"] = node->scene_Pos.x();
            nodeObj["y"] = node->scene_Pos.y();
            nodeObj["color"] = node->brush().color().name();
            nodeObj["label"] = node->labelItem->toPlainText();
            nodesArray.append(nodeObj);
        }
    }

    // Collect edges
    for (QGraphicsItem* item : itemsInScene) {
        EdgeItem* edge = dynamic_cast<EdgeItem*>(item);
        if (edge) {
            QJsonObject edgeObj;
            edgeObj["start_x"] = edge->start->scene_Pos.x();
            edgeObj["start_y"] = edge->start->scene_Pos.y();
            edgeObj["end_x"] = edge->end->scene_Pos.x();
            edgeObj["end_y"] = edge->end->scene_Pos.y();
            edgeObj["weight"] = edge->getWeight();
            edgesArray.append(edgeObj);
        }
    }

    QJsonObject root;
    root["nodes"] = nodesArray;
    root["edges"] = edgesArray;

    // Add radius and global colors
    auto info = staticInformation::instance();
    root["nodeRadius"] = info->nodeR;
    root["nodeColor"] = info->nodeColor.name();
    root["edgeColor"] = info->edgeColor.name();

    QJsonDocument doc(root);
    QString fileName = QFileDialog::getSaveFileName(this, "Export Graph", "", "*.json");
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(doc.toJson());
            file.close();
        }
    }
}

void Graph::importGraph() {
    QString fileName = QFileDialog::getOpenFileName(this, "Import Graph", "", "*.json");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) return;

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject root = doc.object();

    scene->clear();

    QMap<QPair<qreal, qreal>, NodeItem*> positionToNode;

    auto info = staticInformation::instance();
    info->nodeR = root["nodeRadius"].toInt(30);  // fallback default
    info->nodeColor = QColor(root["nodeColor"].toString("#ff0000"));
    info->edgeColor = QColor(root["edgeColor"].toString("#0000ff"));

    // Load nodes
    QJsonArray nodesArray = root["nodes"].toArray();
    for (const QJsonValue& val : nodesArray) {
        QJsonObject obj = val.toObject();
        qreal x = obj["x"].toDouble();
        qreal y = obj["y"].toDouble();
        QString colorStr = obj["color"].toString();
        QString label = obj["label"].toString();

        int nodeR = info->nodeR / 2;
        NodeItem* node = new NodeItem(x - nodeR, y - nodeR, nodeR * 2, nodeR * 2, label);
        node->setBrush(QColor(colorStr));
        scene->addItem(node);
        positionToNode[{x, y}] = node;
    }

    // Load edges
    QJsonArray edgesArray = root["edges"].toArray();
    for (const QJsonValue& val : edgesArray) {
        QJsonObject obj = val.toObject();
        QPointF start(obj["start_x"].toDouble(), obj["start_y"].toDouble());
        QPointF end(obj["end_x"].toDouble(), obj["end_y"].toDouble());


        NodeItem* startNode = positionToNode.value({start.x(), start.y()}, nullptr);
        NodeItem* endNode = positionToNode.value({end.x(), end.y()}, nullptr);

        if (startNode && endNode) {
            double weight = obj["weight"].toDouble(1.0); // default weight
            EdgeItem* edge = new EdgeItem(startNode, endNode, weight);
            //EdgeItem* edge = new EdgeItem(startNode, endNode);
            scene->addItem(edge);
        }
    }

    file.close();
}
