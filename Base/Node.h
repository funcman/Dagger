#ifndef	DAGGER_NODE_H
#define	DAGGER_NODE_H

#include "TypeDef.h"

class DAGGER_API DNode {
public:
	DNode*		prev;
	DNode*		next;
	DWORD		thisPtr;
	DWORD		data;
public:
	DNode();
	bool		IsValid();
	void		SetThis();
	DNode*		GetPrev();
	DNode*		GetNext();
	bool		InsertBefore(DNode* Node);
	bool		InsertAfter(DNode* Node);
	bool		Remove();
	void		Delete();
};

#endif//DAGGER_NODE_H
