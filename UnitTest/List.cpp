#include <gtest/gtest.h>
#include "dList.h"

TEST(ListTest, General) {
    dList* list = new dList();
    ASSERT_TRUE(list!=0);
    dNode* node1 = new dNode();
    ASSERT_TRUE(node1!=0);
    dNode* node2 = new dNode();
    ASSERT_TRUE(node2!=0);
    dNode* node3 = new dNode();
    ASSERT_TRUE(node3!=0);
    dNode* node4 = new dNode();
    ASSERT_TRUE(node4!=0);
    list->AddHead(node1);
    EXPECT_EQ(node1, list->GetHead());
    list->AddTail(node2);
    EXPECT_EQ(node2, list->GetTail());
    list->GetHead()->InsertBefore(node3);
    list->GetTail()->InsertAfter(node4);
    // now: node3-node1-node2-node4
    EXPECT_EQ(node3, list->GetHead());
    EXPECT_EQ(node4, list->GetTail());
    EXPECT_EQ(4, list->GetCount());
    list->GetHead()->Delete();
    EXPECT_EQ(3, list->GetCount());
    EXPECT_EQ(node1, list->GetHead());
    list->GetTail()->Delete();
    EXPECT_EQ(2, list->GetCount());
    EXPECT_EQ(node2, list->GetTail());
    list->Free();
    EXPECT_EQ(0, list->GetCount());
    delete list;
}
