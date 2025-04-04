# Qt Graph Visualizer

This project is a simple interactive **graph editor** built with **Qt (C++/QtWidgets)**. It allows users to add, remove, move, and connect nodes visually, and provides support for import/export of the graph structure using JSON files.

## ✨ Features

- 🟢 Add / remove nodes
- 🧲 Drag and move nodes
- 🔗 Connect nodes with edges
- 🎯 Edge arrows for direction
- 🎨 Customizable node and edge colors
- 📏 Configurable node radius
- 💾 Import/Export graph to JSON

## 📋 Todo

- [x] Add/Remove Nodes and Edges
- [x] Visual edge arrow direction
- [x] Import/Export JSON support
- [ ] Add edge weights
- [ ] Add labels to nodes and edges
- [ ] Highlight selected edges
- [ ] Save and load node colors/radius
- [ ] Implement pathfinding algorithms:
  - [ ] Dijkstra's Algorithm (shortest path)
  - [ ] A* Search Algorithm
- [ ] Implement Minimum Spanning Tree (MST):
  - [ ] Prim's Algorithm
  - [ ] Kruskal's Algorithm
- [ ] Visual animation for algorithm steps
- [ ] Undo/Redo support
- [ ] Search node by ID or label
- [ ] Save as image (SVG, PNG)



## 📸 Preview

![image](https://github.com/user-attachments/assets/8f3e8fd9-e823-4e6a-a73f-cac159d5dac3)


## 🚀 Getting Started

### Prerequisites

- Qt 5 or 6 (tested with Qt 5.15+)
- CMake or qmake
- C++11 or higher

### Build Instructions

Using **CMake**:

```bash
mkdir build
cd build
cmake ..
make
./GraphEditor
```

## Contributing
Feel free to contribute to this project by submitting issues or pull requests. Your feedback and contributions are highly appreciated.

## License

This project is licensed under the GNU General Public License v3.0 - see the [LICENSE](LICENSE) file for details.
