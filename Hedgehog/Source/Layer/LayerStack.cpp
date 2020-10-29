#include <Layer/LayerStack.h>


LayerStack::LayerStack()
{
	overlayStart = 0;
}

// Push layer on top of all layers but before overlays
// insert inserts new element before the element at the specified position
void LayerStack::Push(Layer* const layer)
{
	layers.insert(layers.begin() + overlayStart, layer);
	overlayStart++;
	layer->OnPush();
}

void LayerStack::PushOverlay(Layer* const layer)
{
	layers.push_back(layer);
	layer->OnPush();
}

Layer* LayerStack::Top()
{
	if (overlayStart > 0)
	{
		return *(layers.begin() + overlayStart - 1);
	}
	else
	{
		return NULL;
	}
}

Layer* LayerStack::TopOverlay()
{
	if (layers.size() > overlayStart)
	{
		return layers.back();
	}
	else
	{
		return NULL;
	}
}

void LayerStack::Pop()
{
	Layer* layer = Top();

	if (layer)
	{
		layer->OnPop();
		layers.erase(layers.begin() + overlayStart - 1);
		overlayStart--;
	}
}

void LayerStack::PopOverlay()
{
	Layer* layer = TopOverlay();

	if (layer)
	{
		TopOverlay()->OnPop();
		layers.pop_back();
	}
}
