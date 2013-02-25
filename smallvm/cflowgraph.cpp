#include "cflowgraph.h"

CFlowGraph::CFlowGraph()
{
}

int CFlowGraph::addBasicBlock(int i1, int i2)
{
    BasicBlock bb;
    bb.from = i1;
    bb.to = i2;
    int index = blocks.count();
    bb.index = index;
    blocks.append(bb);

    firstInstructionOf[index] = i1;
    return index;
}

void CFlowGraph::addEdge(int b1, int b2)
{
    flow[b1].insert(b2);
    incomingTo[b2].insert(b1);
}

int CFlowGraph::basicBlockStartingAt(int i)
{
    for(int c=0; c<blocks.count();++c)
    {
        BasicBlock &bb = blocks[c];
        if(bb.from == i)
            return c;
    }
    return -1;
}
