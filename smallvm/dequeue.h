#ifndef DEQUEUE_H
#define DEQUEUE_H

template<class T> struct Node
{
    T data;
    Node *next;
    Node *prev;
    Node(T _data, Node *_next, Node *_prev){
        data = _data; next = _next; prev = _prev;
    }
};

class EmptyDequeException { };

template<class T> class Deque
{
    Node<T> *front;
    Node<T> *back;
public:
    Deque() {
        front = back = NULL;
    }
    void pushFront(T data) {
        if(front == NULL)
        {
            front = back = new Node<T>(data, NULL, NULL);
        }
        else
        {
            Node<T> *nf = new Node<T>(data, front, NULL);
            front->prev = nf;
            front = nf;
        }
    }
    void pushBack(T data) {
        if(back == NULL){
            front = back = new Node<T>(data, NULL, NULL);
        }
        else {
            Node<T> *nb = new Node<T>(data, NULL, back);
            back->next = nb;
            back = nb;
        }
    }
    T popFront() {
        if(front == NULL)
            throw EmptyDequeException();
        Node *of = front;
        front = of->next;
        if(front == NULL)
            back = NULL;
        else
            front->prev = NULL;
        T temp = of->data;
        delete of;
        return temp;
    }

    T popBack() {
        if(back == NULL)
            throw EmptyDequeException();
        Node *ob = back;
        back = ob->prev;
        if(back == NULL)
            front = NULL;
        else
            front->next = NULL;
        T temp = ob->data;
        delete ob;
        return temp;
    }
    bool empty() {
        return front = NULL;
    }

};

#endif // DEQUEUE_H
