#ifndef STACK_H
#define STACK_H

#include <QDebug>

/*
    A faster operand stack specialized for SmallVM's needs, used
    instead of QStack and covers the part of QStack's public interface
    used by SmallVM

    We use a linked list of 'chunks', each of them a small array,
    but the first chunk is allocated statically to cover the common
    case, as most operand stacks store a small number of values.
*/

// Works only for some 'b' that is a power of two
#define modulo(a, b) ((a) & (b-1))
// #define modulo(a, b) ((a) % (b))
template<class T, int Size> struct StackNode
{
    T data[Size];
    StackNode<T, Size> *next, *prev;
    StackNode() : next(0), prev(0) { }
};

template<class T, int ChunkSize> class Stack
{
    StackNode<T, ChunkSize> first;
    StackNode<T, ChunkSize> *last;
    int _count;
    int nchuncks;
    int capacity;
public:
    Stack()
    {
        _count = 0;
        nchuncks = 1;
        last = &first;
        first.next = first.prev = 0;
        capacity = nchuncks * ChunkSize;
    }

    ~Stack()
    {
        while(last != &first)
        {
            removeChunk();
        }
    }

    void clear()
    {
        while(last != &first)
        {
            removeChunk();
        }
        _count = 0;
        nchuncks = 1;
        first.next = first.prev = 0;
    }

    Stack(Stack &other)
    {
        for(Stack<T, ChunkSize>::const_iterator i=other.begin(); i!= other.end(); ++i)
            push(*i);
    }

    Stack &operator=(const Stack &other)
    {
        clear();
        for(Stack<T, ChunkSize>::const_iterator i=other.begin(); i!= other.end(); ++i)
            push(*i);
        return *this;
    }

    void push(T value)
    {
        if(_count == capacity)
        {
            addChunk();
        }
        int n = modulo(_count, ChunkSize);
        last->data[n] = value;
        _count++;
    }
    T pop()
    {
        int n = modulo(_count - 1, ChunkSize);
        T ret = last->data[n];
        /*
        if(n == 0 && (last != &first))
        {
            removeChunk();
        }
        //*/
        _count--;
        return ret;
    }

    inline T &peek()
    {
        int n = modulo(_count, ChunkSize);
        return last->data[n-1];
    }

    inline T &top()
    {
        return peek();
    }

    inline bool isEmpty()
    {
        return _count == 0;
    }

    // Just for QStack compatibility
    void reserve(int) { }

    inline int count() { return _count; }

    inline bool empty() { return isEmpty(); }
    void addChunk()
    {
        //qDebug() << "Chunk added to call stack";
        StackNode<T, ChunkSize> *node = new StackNode<T, ChunkSize>();
        node->next = 0;
        last->next = node;
        node->prev = last;
        last = node;

        nchuncks++;
        capacity = nchuncks * ChunkSize;
    }

    void removeChunk()
    {
        StackNode<T, ChunkSize> *oldLast = last;
        last = oldLast->prev;
        last->next = 0;
        delete oldLast;

        nchuncks--;
        capacity = nchuncks * ChunkSize;
    }

    class const_iterator
    {
    public:
        const Stack<T, ChunkSize> *owner;
        int i_global;

        int i;
        StackNode<T, ChunkSize> *current;

        const_iterator(const Stack<T, ChunkSize> *owner, int i_global):
            owner(owner),
            i_global(i_global)
        {
            current = const_cast<StackNode<T, ChunkSize> *>(&owner->first);
            i = i_global % ChunkSize;
        }

        const_iterator(const const_iterator &other)
        {
            owner = other.owner;
            i = other.i;
            i_global = other.i_global;
            current = other.current;
        }

        inline bool operator !=(const const_iterator &other)
        {
            return (i_global != other.i_global) || (other.owner != owner);
        }

        inline const_iterator &operator++()
        {
            ++i_global;
            ++i;
            if(i == ChunkSize)
            {
                i = 0;
                current = current->next;
            }
            return *this;
        }
        inline const T &operator *()
        {
            return current->data[i];
        }
    };
    friend class const_iterator;
    inline const_iterator begin() const
    {
        return const_iterator(this, 0);
    }

    inline const_iterator end() const
    {
        return const_iterator(this, _count);
    }
};

#endif // STACK_H
