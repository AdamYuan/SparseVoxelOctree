#ifndef SOBOL_HPP
#define SOBOL_HPP

#include <vector>

class Sobol
{
	private:
		unsigned m_index = 0, m_x[10005] = {}, m_dim;
	public:
		Sobol(unsigned dim) : m_dim{dim} {  }
		Sobol() = default;
		void Next(float *out);
		void Reset(unsigned dim) { m_index = 0; m_dim = dim; std::fill(m_x, m_x + dim, 0); }
		void Reset() { m_index = 0; std::fill(m_x, m_x + m_dim, 0); }
		unsigned Dim() const { return m_dim; }
};

#endif
