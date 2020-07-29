#include "Sobol.hpp"
#include "Sobol.inl"

/*static inline uint8_t get_first_zero_bit(unsigned x) {
	uint8_t c = kTable[x & 255];
	if (c != 8) return c;
	c += kTable[(x >> 8) & 255];
	if (c != 16) return c;
	c += kTable[(x >> 16) & 255];
	if (c != 24) return c;
	return c + kTable[(x >> 24) & 255];
}

void Sobol::Next(float *out) {
	uint8_t c = get_first_zero_bit(m_index++);
	//uint8_t c = glm::findLSB(~(m_index ++));
	for (unsigned j = 0; j < m_dim; ++j)
		out[j] = (float) ((m_x[j] ^= kMatrices[j][c]) / 4294967296.0);
}*/
void Sobol::Initialize(const std::shared_ptr<myvk::Device> &device) {

}
