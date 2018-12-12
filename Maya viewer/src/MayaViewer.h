#ifndef MayaViewer_H_
#define MayaViewer_H_

#include "gameplay.h"
#include "comLib.h"
#include "Data.h"


using namespace gameplay;

/**
 * Main game class.
 */
class MayaViewer: public Game
{

private:

	/**
	* Draws the scene each frame.
	*/
	bool drawScene(Node* node);

	bool addMesh();
	void createMesh(char* data);
	void transformMesh(char* data);
	void scaleMesh(char* data);
	void rotateMesh(char * data);
	void updateCamera(char * data);

	Scene* _scene;
	bool _wireframe;

	bool onceBool = true;
	bool test();
	bool test2 = false;

	void getMayaData();

	bool MayaViewer::initializeMaterials(Node* node);
	comLib comlib;

	struct meshList {
		int id;
		string meshName;
	};

	int nrOfAddedMeshes = 0;

	vector<meshList> mVectorList;
public:

    /**
     * Constructor.
     */
    MayaViewer();

    /**
     * @see Game::keyEvent
     */
	void keyEvent(Keyboard::KeyEvent evt, int key);
	
    /**
     * @see Game::touchEvent
     */
    void touchEvent(Touch::TouchEvent evt, int x, int y, unsigned int contactIndex);

protected:

    /**
     * @see Game::initialize
     */
    void initialize();

    /**
     * @see Game::finalize
     */
    void finalize();

    /**
     * @see Game::update
     */
    void update(float elapsedTime);

    /**
     * @see Game::render
     */
    void render(float elapsedTime);


};

#endif
