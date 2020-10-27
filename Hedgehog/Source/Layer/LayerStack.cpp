#include <Layer/LayerStack.h>


LayerStack::LayerStack()
{
	overlayStart = layers.begin();
}

// Push layer on top of all layers but before overlays
// deque::insert inserts new element before the element at the specified position
void LayerStack::Push(Layer* const layer)
{
	overlayStart = layers.insert(overlayStart, layer);
	layer->OnPush();
}

void LayerStack::PushOverlay(Layer* const layer)
{
	layers.push_back(layer);
	layer->OnPush();
}

Layer* LayerStack::Top()
{
	return *(overlayStart - 1);
}

Layer* LayerStack::TopOverlay()
{
	return layers.back();
}

void LayerStack::Pop()
{
	Top()->OnPop();
	layers.erase(overlayStart - 1);
}

void LayerStack::PopOverlay()
{
	TopOverlay()->OnPop();
	layers.pop_back();
}
