#ifndef FRAMEPOOL_H
#define FRAMEPOOL_H

#include "frame.h"
#include "stack.h"

class FramePool
{
    Stack<Frame *> freeList;
public:
    inline Frame *allocate(Method *method, int stackLevel)
    {
        if(!freeList.empty())
        {
            Frame *f = freeList.pop();
            f->Init(method, 0, stackLevel);
            return f;
        }
        else
        {
            return new Frame(method, stackLevel);
        }
    }
    inline void free(Frame *f)
    {
        freeList.push(f);
    }
    ~FramePool()
    {
        for(Stack<Frame *>::const_iterator iter = freeList.begin(); iter != freeList.end(); ++iter)
        {
            Frame *f = *iter;
            delete f;
        }
    }
};

#endif // FRAMEPOOL_H
