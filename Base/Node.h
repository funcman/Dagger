#ifndef	DAGGER_NODE_H
#define	DAGGER_NODE_H

#include <stdint.h>

#include "TypeDef.h"

class DAGGER_API DNode {
public:
	DNode*		prev;
	DNode*		next;
	uintptr_t	thisPtr;
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
