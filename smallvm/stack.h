#ifndef STACK_H
#define STACK_H

/*
    A faster operand stack specialized for SmallVM's needs, used
    instead of QStack and covers the part of QStack's public interface
    used by SmallVM

    We use a linked list of 'chunks', each of them a small array,
    but the first chunk is allocated statically to cover the common
    case, as most operand stacks store a small number of values.
*/

template<class T, int Size> struct StackNode
{
    T data[Size];
    StackNode<T, Size> *next, *prev;
    StackNode() : next(0), prev(0) { }
};

const int ChunkSize=20;
template<class T> class Stack
{
    StackNode<T, ChunkSize> first;
    StackNode<T, ChunkSize> *last;
    int _count;
    int nchuncks;
public:
    Stack()
    {
        _count = 0;
        nchuncks = 1;
        last = &first;
        first.next = first.prev = 0;
    }

    ~Stack()
    {
        while(last != &first)
        {
            removeChunk();
        }
    }

    Stack(Stack &other)
    {
        for(Stack<T>::const_iterator i=other.begin(); i!= other.end(); ++i)
            push(*i);
    }

    Stack &operator=(const Stack &other)
    {
        while(!empty())
            pop();
        for(Stack<T>::const_iterator i=other.begin(); i!= other.end(); ++i)
            push(*i);
        return *this;
    }

    void push(T value)
    {
        if(_count == (nchuncks *ChunkSize))
        {
            addChunk();
        }
        int n = _count % ChunkSize;
        last->data[n] = value;
        _count++;
    }
    T pop()
    {
        int n = _count % ChunkSize;
        T ret = last->data[n-1];
        n--;
        if(n == 0 && (last != &first))
        {
            removeChunk();
        }
        _count--;
        return ret;
    }

    inline T &peek()
    {
        int n = _count % ChunkSize;
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

    inline int count() { return _count; }

    inline bool empty() { return isEmpty(); }
    void addChunk()
    {
        StackNode<T, ChunkSize> *node = new StackNode<T, ChunkSize>();
        node->next = 0;
        last->next = node;
        node->prev = last;
        last = node;
    }
    void removeChunk()
    {
        StackNode<T, ChunkSize> *oldLast = last;
        last = oldLast->prev;
        last->next = 0;
        delete oldLast;
    }

    class const_iterator
    {
    public:
        const Stack<T> *owner;
        int i_global;

        int i;
        StackNode<T, ChunkSize> *current;

        const_iterator(const Stack<T> *owner, int i_global):
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
