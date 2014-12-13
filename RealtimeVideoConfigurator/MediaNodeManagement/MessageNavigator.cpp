#include "MessageNavigator.h"

MessageNavigator::MessageNavigator(BMessage *treeMessage)
{
	nodeTree=treeMessage;
	Init();
}

void MessageNavigator::Init(void)
{
	queue			= new BMessageQueue();
	saveQueue		= new BMessageQueue();
	if  (nodeTree == NULL)
		nodeTree	 = new BMessage();
	AddBranch(nodeTree);
	saveQueue=queue;
	currentOutput 	= NULL;
	currentInput	= NULL;
}

media_node* MessageNavigator::GetCurrentOutputNode(void)
{
	return currentOutput;
}
media_node* MessageNavigator::GetNextInput(void)
{

	ssize_t size;
	//the old Input becomes now the current Output
	currentOutput = currentInput;
	//get the next Node from the Queue
	BMessage *tmpMessage = queue->NextMessage();
	if (tmpMessage->FindData("media_node",B_RAW_TYPE,(const void **)&currentInput,&size) == B_OK)
		return currentInput;
	else
		return NULL;
}
/*status_t MessageNavigator::AddBranch(BMessage *branch)
{
	BMessage *tmpMessage=new BMessage();
	BMessage *parent=new BMessage();
	int32 i=0;
	status_t err=B_OK;
	while ((!err))
	{
		err = branch->FindMessage("Node",i,tmpMessage);
		if (tmpMessage!=NULL)
		{
			if (tmpMessage->what==MEDIA_NODE)
			{
			}
			else if (tmpMessage->what==METHA_NODE)
			{
				
			}
			
		}
		else
			err=B_BAD_INDEX;
	}
	queue->
	branch
}*/

status_t MessageNavigator::AddBranch(BMessage *branch)
{
	status_t		err 	= B_OK;
	int 			i		= 0;
	BMessage *tmpMessage	= new BMessage();
	bool			added	= false;
	if (branch->what==MEDIA_NODE)
	{
		queue->AddMessage(branch);
		added=true;
		while ((!err))
		{
			err = branch->FindMessage("Node",i,tmpMessage);			
			if (tmpMessage!=NULL)
			{
				if (tmpMessage->what == METHA_NODE)
				{
					if (!added) queue->AddMessage(branch);
					AddBranch(tmpMessage);
					added=false;
				}
			
			}
			else
				err=B_BAD_INDEX;
		}

	}
	else if (branch->what == METHA_NODE)
	{
		while ((!err))
		{
			err = branch->FindMessage("Node",i,tmpMessage);
			if (tmpMessage!=NULL)
			{
				AddBranch(tmpMessage);
			}
			else
				err=B_BAD_INDEX;
		}
	}
	return B_OK;
}

