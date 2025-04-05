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

void GraphScene::runDijkstra(NodeItem* start, NodeItem* end) {
    QMap<NodeItem*, qreal> distances;
    QMap<NodeItem*, NodeItem*> previous;
    QSet<NodeItem*> visited;
    QList<NodeItem*> nodes;

    for (QGraphicsItem* item : items()) {
        auto node = dynamic_cast<NodeItem*>(item);
        if (node) {
            distances[node] = std::numeric_limits<qreal>::max();
            nodes.append(node);
        }
    }

    distances[start] = 0;

    while (!nodes.isEmpty()) {
        std::sort(nodes.begin(), nodes.end(), [&](NodeItem* a, NodeItem* b) {
            return distances[a] < distances[b];
        });

        NodeItem* current = nodes.takeFirst();
        if (current == end) break;
        if (visited.contains(current)) continue;

        visited.insert(current);

        //qDebug() << current->labelItem->toPlainText();
        for (EdgeItem* edge : current->connectedEdges) {
            NodeItem* neighbor = (edge->start == current) ? edge->end : nullptr;
            if(!neighbor) continue;
            if (visited.contains(neighbor)) continue;

            qreal alt = distances[current] + edge->getWeight();
            if (alt < distances[neighbor]) {
                distances[neighbor] = alt;
                previous[neighbor] = current;
            }
        }
    }

    // Highlight the shortest path
    NodeItem* current = end;
    while (previous.contains(current)) {
        NodeItem* prev = previous[current];

        for (EdgeItem* edge : current->connectedEdges) {
            if ((edge->start == current && edge->end == prev) ||
                (edge->start == prev && edge->end == current)) {
                edge->setPen(Qt::green, 3);
                break;
            }
        }

        current = prev;
    }
}

void GraphScene::runA_Start(NodeItem* start, NodeItem* end) {
    QHash<NodeItem*, qreal> gScore;
    QHash<NodeItem*, qreal> fScore;
    QHash<NodeItem*, NodeItem*> cameFrom;
    QSet<NodeItem*> openSet;
    QList<NodeItem*> openList;

    // Initialize all nodes
    for (QGraphicsItem* item : items()) {
        NodeItem* node = dynamic_cast<NodeItem*>(item);
        if (node) {
            gScore[node] = std::numeric_limits<qreal>::max(); // Initialize gScore as infinity
            fScore[node] = std::numeric_limits<qreal>::max(); // Initialize fScore as infinity
        }
    }

    gScore[start] = 0;
    fScore[start] = 0;  // No heuristic, just set to 0
    openSet.insert(start);  // Add start node to the open set
    openList.append(start); // Add start node to the open list

    while (!openList.isEmpty()) {
        // Sort open list based on fScore
        std::sort(openList.begin(), openList.end(), [&](NodeItem* a, NodeItem* b) {
            return fScore[a] < fScore[b]; // Compare by fScore (only gScore here, no heuristic)
        });

        NodeItem* current = openList.takeFirst();  // Get node with lowest fScore
        openSet.remove(current); // Remove it from open set

        if (current == end) break; // We found the shortest path

        // Explore neighbors
        for (EdgeItem* edge : current->connectedEdges) {
            NodeItem* neighbor = (edge->start == current) ? edge->end : nullptr;
            if(!neighbor) continue;
            qreal tentative_gScore = gScore[current] + edge->getWeight();  // Calculate the cost to reach the neighbor

            // Only consider this neighbor if the new path to it is better
            if (tentative_gScore < gScore[neighbor]) {
                cameFrom[neighbor] = current;
                gScore[neighbor] = tentative_gScore;
                fScore[neighbor] = gScore[neighbor]; // Since there's no heuristic, fScore is just gScore

                // Only add to openList if it's not already in openSet
                if (!openSet.contains(neighbor)) {
                    openSet.insert(neighbor);
                    openList.append(neighbor);  // Add neighbor to open list
                }
            }
        }
    }

    // Reconstruct and highlight the path
    NodeItem* current = end;
    while (cameFrom.contains(current)) {
        NodeItem* prev = cameFrom[current];

        // Highlight the edge
        for (EdgeItem* edge : current->connectedEdges) {
            if ((edge->start == current && edge->end == prev) ||
                (edge->start == prev && edge->end == current)) {
                edge->setPen(Qt::darkMagenta, 3); // Highlight the path
                break;
            }
        }

        current = prev;
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

            startNode->connectedEdges.append(edge);
            startNode->neighbors.append(endNode);
            //endNode->connectedEdges.append(edge);
            //endNode->neighbors.append(startNode);

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

    QPushButton* btnDijkstra = new QPushButton("Run Dijkstra");
    connect(btnDijkstra, &QPushButton::clicked, this, [=]() {
        QList<QGraphicsItem*> allItems = scene->items();
        QString n1_label = QInputDialog::getText(nullptr, "Node Label Start", "Enter node label Start:");
        QString n2_label = QInputDialog::getText(nullptr, "Node Label End", "Enter node label End:");
        NodeItem* n1 = nullptr;
        NodeItem* n2 = nullptr;
        for(QGraphicsItem* item: allItems){
            NodeItem* item_n = dynamic_cast<NodeItem*>(item);
            EdgeItem* item_e = dynamic_cast<EdgeItem*>(item);
            auto info = staticInformation::instance();
            if(item_e) item_e->setPen(info->edgeColor, 2);
            if(!item_n) continue;
            if(item_n->labelItem->toPlainText() == n1_label){
                n1 = item_n;
            }
            if(item_n->labelItem->toPlainText() == n2_label){
                n2 = item_n;
            }
        }
        if (n1 && n2) {
            scene->runDijkstra(n1, n2);
        } else {
            QMessageBox::information(this, "Dijkstra", "Please select 2 valid nodes.");
        }
    });

    QPushButton* btnA_Start = new QPushButton("Run A*");
    connect(btnA_Start, &QPushButton::clicked, this, [=]() {
        QList<QGraphicsItem*> allItems = scene->items();
        QString n1_label = QInputDialog::getText(nullptr, "Node Label Start", "Enter node label Start:");
        QString n2_label = QInputDialog::getText(nullptr, "Node Label End", "Enter node label End:");
        NodeItem* n1 = nullptr;
        NodeItem* n2 = nullptr;
        for(QGraphicsItem* item: allItems){
            NodeItem* item_n = dynamic_cast<NodeItem*>(item);
            EdgeItem* item_e = dynamic_cast<EdgeItem*>(item);
            auto info = staticInformation::instance();
            if(item_e) item_e->setPen(info->edgeColor, 2);
            if(!item_n) continue;
            if(item_n->labelItem->toPlainText() == n1_label){
                n1 = item_n;
            }
            if(item_n->labelItem->toPlainText() == n2_label){
                n2 = item_n;
            }
        }
        if (n1 && n2) {
            scene->runA_Start(n1, n2);
        } else {
            QMessageBox::information(this, "Dijkstra", "Please select 2 valid nodes.");
        }
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

    layTop->addWidget(btnDijkstra, 2, 0);
    layTop->addWidget(btnA_Start, 2, 1);

    scene = new GraphScene(stateMouse, this);
    view = new QGraphicsView(scene, this);

    laymain->addLayout(layTop);
    laymain->addWidget(view);
    setLayout(laymain);
    //scene->setSceneRect(0, 0, 800, 600);
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

            startNode->connectedEdges.append(edge);
            startNode->neighbors.append(endNode);
            endNode->connectedEdges.append(edge);
            endNode->neighbors.append(startNode);

            scene->addItem(edge);
        }
    }

    file.close();
}
