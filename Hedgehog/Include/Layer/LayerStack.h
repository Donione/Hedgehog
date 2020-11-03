#pragma once

#include <Layer/Layer.h>

#include <vector>


// LayerStack doesn't own Layers that are pushed into it
// Meaning that it doesn't clean up on Pops or in destructor
// TODO: Might be useful to use smart pointers
//
//    E | | Overlay pushed last  | ^ r   <--- end()
//    v | | Overlays in between  | | e
//    e | | Overlay pushed first | | d   <--- overlayStart
//    n | | Layer pushed last    | | n
//    t | | Layers in between    | | e
//    s V | Layer pushed first   | | R   <--- begin()
class LayerStack
{
public:
	LayerStack();

	void Push(Layer* const layer);
	void PushOverlay(Layer* const layer);
	Layer* Top();
	Layer* TopOverlay();
	void Pop();
	void PopOverlay();

	std::vector<Layer*>::iterator begin() { return layers.begin(); }
	std::vector<Layer*>::iterator end() { return layers.end(); }
	std::vector<Layer*>::reverse_iterator rbegin() { return layers.rbegin(); }
	std::vector<Layer*>::reverse_iterator rend() { return layers.rend(); }

private:
	// Maybe a std::list would be better
	std::vector<Layer*> layers;

	// Turns out that keeping an iterator to index something is not the best idea,
	// the iterator is invalidated if there is a re-allocation for the collection
	// So instead of this:
	//    std::vector<Layer*>::iterator overlayStart;
	// it's better to use an index and iterator arithmetics
	unsigned int overlayStart;
};