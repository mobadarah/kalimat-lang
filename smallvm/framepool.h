#ifndef FRAMEPOOL_H
#define FRAMEPOOL_H

#include "frame.h"
#include "stack.h"

class FramePool
{
    static const int FramePoolSize = 32; // must be a power of two!
    Stack<Frame *, FramePoolSize> freeList;
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
        for(Stack<Frame *, FramePoolSize>::const_iterator iter = freeList.begin(); iter != freeList.end(); ++iter)
        {
            Frame *f = *iter;
            delete f;
        }
    }
};

#endif // FRAMEPOOL_H
