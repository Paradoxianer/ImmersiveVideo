#ifndef NODE_CONTROL_VIEW_H
#define NODE_CONTROL_VIEW_H

class NodeControlView
{

public:
						NodeControlView();
						~NodeControlView();
		media_node*		GetOutputMediaNodes(void);
		media_node*		GetInputMediaNodes(void);
		void			SetMediaRoster(BMediaRoster *r){mediaRoster=r};


protected:
		BMediaRoster*	mediaRoster;

private:
};
#endif
