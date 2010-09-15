/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#include "statemachine_incl.h"

void StateMachine::add(State s, Predicate *c, State s2)
{
        _transitions[s].push_back(TransitionTestPair(c, s2));
}
void StateMachine::add(State s, Predicate *c, State s2, Action *action)
{
        _transitions[s].push_back(TransitionTestPair(c, s2, action));
}
void StateMachine::addAccepting(State s, Predicate *c, State s2, TokenType accepted)
{
    _transitions[s].push_back(TransitionTestPair(c, s2, accepted));
}
void StateMachine::addAccepting(State s, Predicate *c, State s2, TokenType accepted, Action *action)
{
    _transitions[s].push_back(TransitionTestPair(c, s2, accepted, action));
}

bool StateMachine::Transition(State s, Buffer &_buffer, State &nextState,
                              bool &accept, TokenType & acceptedType, Action *&action)
{
        if(StateHasTests(s))
        {
                TransitionTestMap allTests = _transitions[s];

                for(TransitionTestMap::iterator p = allTests.begin(); p!=allTests.end(); ++p)
                {
                        Predicate *test = (*p).c;
                        if((*test)(_buffer))
                        {
                                nextState = (*p).s;
                                action = (*p).action;
                                accept = (*p).accepting;
                                acceptedType = (*p).acceptedType;
                                return true;
                        }
                }

                return false;
        }
        return false;

}

bool StateMachine::StateHasTests(State s)
{

        TransitionTable::iterator i = _transitions.find(s);
        return i != _transitions.end();
}
QVector<Predicate *> StateMachine::GetPossibleTransitions (State s)
{

        QVector<Predicate *> ret;
        TransitionTable::iterator p = _transitions.find(s);
        if(p== _transitions.end())
        {
                return ret;
        }
        TransitionTestMap m= _transitions[s];
        TransitionTestMap::iterator p2;
        for(p2 = m.begin(); p2!=m.end(); ++p2)
        {
                Predicate *cp= (*p2).c;
                ret.push_back(cp);
        }
        return ret;

}
TransitionTestPair::TransitionTestPair()
{
}
TransitionTestPair::TransitionTestPair(Predicate *c1, State s1)
{
        c= c1;
        s= s1;
        action = NULL;
        accepting = false;
}
TransitionTestPair::TransitionTestPair(Predicate *c1, State s1, Action *_action)
{
        c= c1;
        s= s1;
        action = _action;

}
TransitionTestPair::TransitionTestPair(Predicate *c1, State s1, TokenType _acceptedType)
{
        c= c1;
        s= s1;
        action = NULL;
        accepting = true;
        acceptedType = _acceptedType;
}
TransitionTestPair::TransitionTestPair(Predicate *c1, State s1, TokenType _acceptedType, Action *_action)
{
        c= c1;
        s= s1;
        action = _action;
        accepting = true;
        acceptedType = _acceptedType;
}
