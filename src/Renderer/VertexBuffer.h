#pragma once

class VertexBuffer {
private:
	unsigned int rendererId;
public:
	VertexBuffer(const void* data, unsigned int size);
	VertexBuffer() = default;
	void Dispose() const;
	void Bind() const;
	void Unbind() const;
	int GetRendererId() {
		return rendererId;
	}
};