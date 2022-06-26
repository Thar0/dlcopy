#ifndef VECTOR_H_
#define VECTOR_H_

#include <stddef.h>

typedef struct Vector
{
    size_t elemSize;
    size_t limit;
    size_t capacity;
    void* start;
    void* end;
    void* rstart;
    void* rend;
} Vector;

#define VECTOR_FOR_EACH_ELEMENT(vector, element) \
    for ((element) = (vector)->start; (element) != (vector)->end; (element)++)

void
Vector_New (Vector* vector, size_t elemSize);

int
Vector_Destroy (Vector* vector);

void*
Vector_At (const Vector* vector, size_t pos);

int
Vector_Reserve (Vector* vector, size_t num);

void*
Vector_Insert (Vector* vector, size_t position, size_t num, const void* data);

void*
Vector_PushBack (Vector* vector, size_t num, const void* data);

int
Vector_Delete (Vector* vector, size_t position, size_t num);

int
Vector_Resize (Vector* vector);

void*
Vector_Release (Vector* vector);

void
Vector_Clear (Vector* vector);

#endif
