#pragma once

class VertexBuffer {
private:
	
	unsigned int rendererId;
public:
	void* data;
	unsigned int size;
	VertexBuffer(void* data, unsigned int size);
	VertexBuffer() = default;
	void Dispose() const;
	void Bind() const;
	void Update(void* data, unsigned int size) const;
	void Unbind() const;
	int GetRendererId() {
		return rendererId;
	}
};