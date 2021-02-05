#pragma once

class IndexBuffer {
private:
	unsigned int rendererId;
	unsigned int count;
public:
	IndexBuffer(const unsigned int* data, unsigned int count);
	void Dispose() const;
	void Bind() const;
	void Unbind() const;
	inline unsigned int GetCount() const {return count;}
};