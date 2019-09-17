////////////////////////////////////////////////////////////////////////
// All keyboard, mouse, and other interactions are implemented here.
// The single entry point, InitInteraction, sets up GLUT callbacks for
// various events that an interactive graphics program needs to
// handle.
//
// The various callbacks are:
//    glutDisplayFunc: called repeatedly to redraw the scene every 30th of a second.
//    glutReshapeFunc: called whenever the window changes size
//    glutKeyboardFunc and glutKeyboardUpFunc: called whenever a key is pressed or released
//    glutMouseFunc:  called when a mouse button is pressed or released
//    glutMotionFunc: called when the mouse is moved

void InitInteraction();
