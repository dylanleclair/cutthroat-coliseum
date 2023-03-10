// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//  * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//  * Neither the name of NVIDIA CORPORATION nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Copyright (c) 2008-2022 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  

#ifndef PHYSX_SNIPPET_RENDER_H
#define PHYSX_SNIPPET_RENDER_H

#include "PxPhysicsAPI.h"
#include "PxPreprocessor.h"

#if PX_WINDOWS
	#include <windows.h>
	#pragma warning(disable: 4505)
	#include <GL/glew.h>
#elif PX_LINUX_FAMILY
	#if PX_CLANG
		#pragma clang diagnostic push
		#pragma clang diagnostic ignored "-Wreserved-identifier"
	#endif
	#include <GL/glew.h>
	#include <GL/freeglut.h>	
	#if PX_CLANG
		#pragma clang diagnostic pop
	#endif
#elif PX_OSX
	#include <GL/glew.h>
	#include <GLUT/glut.h>	
#else
	#error platform not supported.
#endif

typedef	void	(*KeyboardCallback)	(unsigned char key, const physx::PxTransform& camera);
typedef	void	(*RenderCallback)	();
typedef	void	(*ExitCallback)		();

namespace Snippets
{
	class Camera;

	class TriggerRender
	{
		public:
		virtual	bool	isTrigger(physx::PxShape*)	const	= 0;
	};

#if PX_SUPPORT_GPU_PHYSX
	class SharedGLBuffer
	{
	private:
		physx::PxCudaContextManager* cudaContextManager;
		void* vbo_res;
		void* devicePointer;
	public:
		GLuint vbo; //Opengl vertex buffer object
		physx::PxU32 size;
	};
#endif

	void	renderActors(physx::PxRigidActor** actors, const physx::PxU32 numActors, GLuint modelUniform);
}

#endif //PHYSX_SNIPPET_RENDER_H
