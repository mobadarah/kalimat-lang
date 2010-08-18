/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#ifndef MULTIDIMENSIONALARRAY_H
#define MULTIDIMENSIONALARRAY_H

#include <QVector>
template <typename T> class MultiDimensionalArray
{
public:
    QVector<int> dimensions;
    QVector<T> elements;
public:
    MultiDimensionalArray(QVector<int> dimensions);
    const T &get(QVector<int> index);
    void set(QVector<int> index, const T &value);
private:
    int calculatePosition(QVector<int> index, QVector<int> dimensions);
    int product(QVector<int> nums);

};


template <typename T>
MultiDimensionalArray<T>::MultiDimensionalArray(QVector<int> _dimensions)
    :dimensions(_dimensions), elements(product(_dimensions))
{

}

template <typename T>
        const T &MultiDimensionalArray<T>::get(QVector<int>index)
{
    int loc = calculatePosition(index, dimensions);
    return elements[loc];
}

template <typename T> void MultiDimensionalArray<T>::set(QVector<int> index, const T &value)
{
    int loc = calculatePosition(index, dimensions);
    elements[loc] = value;
}

template <typename T> int MultiDimensionalArray<T>::product(QVector<int> nums)
{
    int ret = 1;
    for(int i=0; i<nums.count(); i++)
        ret *= nums[i];
    return ret;
}
template <typename T> int MultiDimensionalArray<T>::calculatePosition(QVector<int>index, QVector<int>dimensions)
{
    Q_ASSERT_X(index.count() == dimensions.count(), "MultiDimensionalArray::get", "wrong number of dimensions in index");
    Q_ASSERT_X(index[0]>=0 && index[0]<dimensions[0], "MultiDimensionalArray::get", "zeroth index out of range");
    int loc = index[0];
    for(int i=1; i<dimensions.count(); i++)
    {
        Q_ASSERT_X(index[i]>=0 && index[i]<dimensions[i], "MultiDimensionalArray::get", "index out of range");
        loc += index[i] * dimensions[i-1];
    }
    return loc;
}

#endif // MULTIDIMENSIONALARRAY_H
