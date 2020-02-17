///////////////////////////////////////////////////////////////////////
// A slight encapsulation of a Frame Buffer Object (i'e' Render
// Target) and its associated texture.  When the FBO is "Bound", the
// output of the graphics pipeline is captured into the texture.  When
// it is "Unbound", the texture is available for use as any normal
// texture.
////////////////////////////////////////////////////////////////////////
class FBO {
public:
    unsigned int fboID;
    unsigned int textureID;
    unsigned int gPosition;
	unsigned int gNormal;
	unsigned int gAlbedoSpec;
	unsigned int Diffuse;
    int width, height;  // Size of the texture.

    void CreateFBO(const int w, const int h);
	void CreateG(const int w, const int h);
    void Bind();
    void Unbind();
	
};
