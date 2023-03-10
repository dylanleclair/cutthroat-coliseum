#include "IndexBuffer.h"

#include <utility>


IndexBuffer::IndexBuffer()
	: bufferID{}
{
	bind();
}


void IndexBuffer::uploadData(GLsizeiptr size, const void* data, GLenum usage) {
	bind();
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, usage);
}
