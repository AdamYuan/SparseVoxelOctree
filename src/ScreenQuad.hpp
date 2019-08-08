//
// Created by adamyuan on 19-6-28.
//

#ifndef DEEPGBUFFERGI_SCREENQUAD_HPP
#define DEEPGBUFFERGI_SCREENQUAD_HPP

#include <mygl3/buffer.hpp>
#include <mygl3/vertexarray.hpp>

class ScreenQuad
{
private:
	mygl3::Buffer m_vbo;
	mygl3::VertexArray m_vao;
public:
	void Initialize();
	void Render() const;
};


#endif //DEEPGBUFFERGI_SCREENQUAD_HPP
