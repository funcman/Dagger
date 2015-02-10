#include "dNode.h"
#include "dBuffer.h"

dNode::dNode() {
    next_ = 0;
    prev_ = 0;
    this_ = 0;
}

bool dNode::IsValid() {
    return this_ && (this_==(long)next_+(long)prev_);
}

void dNode::SetThis() {
    this_ = (long)next_ + (long)prev_;
}

dNode* dNode::GetPrev() {
    if (prev_ && IsValid()) {
        if (prev_->IsValid() && prev_->prev_) {
            return prev_;
        }
    }
    return 0;
}

dNode* dNode::GetNext() {
    if (next_ && IsValid()) {
        if (next_->IsValid() && next_->next_) {
            return next_;
        }
    }
    return 0;
}

void dNode::SetPrev(dNode* node) {
    prev_ = node;
}

void dNode::SetNext(dNode* node) {
    next_ = node;
}

bool dNode::InsertBefore(dNode* node) {
    if (prev_ && IsValid()) {
        if (prev_->IsValid() && node->IsValid()) {
            node->Remove();
            node->prev_ = prev_;
            node->next_ = this;
            node->SetThis();
            prev_->next_ = node;
            prev_->SetThis();
            prev_ = node;
            SetThis();
            return true;
        }
    }
    return false;
}

bool dNode::InsertAfter(dNode* node) {
    if (next_ && IsValid()) {
        if (next_->IsValid() && node->IsValid()) {
            node->Remove();
            node->prev_ = this;
            node->next_ = next_;
            node->SetThis();
            next_->prev_ = node;
            next_->SetThis();
            next_ = node;
            SetThis();
            return true;
        }
    }
    return false;
}

bool dNode::Remove() {
    if (prev_ && next_ && IsValid()) {
        if (prev_->IsValid() && next_->IsValid()) {
            prev_->next_ = next_;
            prev_->SetThis();
            next_->prev_ = prev_;
            next_->SetThis();
            prev_ = 0;
            next_ = 0;
            SetThis();
            return true;
        }
    }
    return false;
}

void dNode::Delete() {
    Remove();
    dFree(this);
}
