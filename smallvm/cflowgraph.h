#ifndef CFLOWGRAPH_H
#define CFLOWGRAPH_H

#include <QVector>
#include <QMap>
#include <QSet>

struct BasicBlock
{
    int index;
    int from;
    int to;
    int stackChange;
};

class CFlowGraph
{
    QVector<BasicBlock> blocks;
    QMap<int, QSet<int> > flow; // from block index (in the 'blocks' vector) to block index
    QMap<int, QSet<int> > incomingTo; // maps from block index to indices of all blocks coming into it
    QMap<int, int> firstInstructionOf; // maps from block index to instruction index
public:
    CFlowGraph();
    int addBasicBlock(int i1, int i2);
    void addEdge(int b1, int b2);
    int basicBlockStartingAt(int i);
    int basicBlockCount() { return blocks.count(); }
    BasicBlock &basicBlock(int i) { return blocks[i]; }
};

#endif // CFLOWGRAPH_H
