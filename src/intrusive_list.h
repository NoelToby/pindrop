// Copyright 2015 Google Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef PINDROP_INTRUSIVE_LIST_H_
#define PINDROP_INTRUSIVE_LIST_H_

#include <cstdint>
#include <cassert>

namespace pindrop {

// Whether to enable IntrusiveList::ValidateList().  Be careful when enabling
// this since this changes the size of IntrusiveListNode so make sure *all*
// projects that include intrusive_list.h also define this value in the same way
// to avoid data corruption.
#ifndef PINDROP_INTRUSIVE_LIST_VALIDATE
#define PINDROP_INTRUSIVE_LIST_VALIDATE 0
#endif  // PINDROP_INTRUSIVE_LIST_VALIDATE

// IntrusiveListNode is used to implement an intrusive doubly-linked
// list.
//
// For example:
//
// class MyClass {
// public:
//   MyClass(const char *msg) : msg_(msg) {}
//   const char* GetMessage() const { return msg_; }
//   PINDROP_INTRUSIVE_GET_NODE(node_);
//   PINDROP_INTRUSIVE_LIST_NODE_GET_CLASS(MyClass, node_);
// private:
//   IntrusiveListNode node_;
//   const char *msg_;
// };
//
// int main(int argc, char *argv[]) {
//   IntrusiveListNode list; // NOTE: type is NOT MyClass
//   MyClass a("this");
//   MyClass b("is");
//   MyClass c("a");
//   MyClass d("test");
//   list.InsertBefore(a.GetListNode());
//   list.InsertBefore(b.GetListNode());
//   list.InsertBefore(c.GetListNode());
//   list.InsertBefore(d.GetListNode());
//   for (IntrusiveListNode* node = list.GetNext();
//        node != list.GetTerminator(); node = node->GetNext()) {
//     MyClass *cls = MyClass::GetInstanceFromListNode(node);
//     printf("%s\n", cls->GetMessage());
//   }
//   return 0;
// }
class IntrusiveListNode {
 public:
  // Initialize the node.
  IntrusiveListNode() {
    Initialize();
#if PINDROP_INTRUSIVE_LIST_VALIDATE
    magic_ = k_magic;
#endif  // PINDROP_INTRUSIVE_LIST_VALIDATE
  }

  // If the node is in a list, remove it from the list.
  ~IntrusiveListNode() {
    Remove();
#if PINDROP_INTRUSIVE_LIST_VALIDATE
    magic_ = 0;
#endif  // PINDROP_INTRUSIVE_LIST_VALIDATE
  }

  // Insert this node after the specified node.
  void InsertAfter(IntrusiveListNode* const node) {
    assert(!node->InList());
    node->next_ = next_;
    node->prev_ = this;
    next_->prev_ = node;
    next_ = node;
  }

  // Insert this node before the specified node.
  void InsertBefore(IntrusiveListNode* const node) {
    assert(!node->InList());
    node->next_ = this;
    node->prev_ = prev_;
    prev_->next_ = node;
    prev_ = node;
  }

  // Get the terminator of the list.
  const IntrusiveListNode* GetTerminator() const { return this; }

  // Remove this node from the list it's currently in.
  IntrusiveListNode* Remove() {
    prev_->next_ = next_;
    next_->prev_ = prev_;
    Initialize();
    return this;
  }

  // Determine whether this list is empty or the node isn't in a list.
  bool IsEmpty() const { return GetNext() == this; }

  // Determine whether this node is in a list or the list contains nodes.
  bool InList() const { return !IsEmpty(); }

  // Calculate the length of the list.
  std::uint32_t GetLength() const {
    std::uint32_t length = 0;
    const IntrusiveListNode* const terminator = GetTerminator();
    for (const IntrusiveListNode* node = GetNext(); node != terminator;
         node = node->GetNext()) {
      length++;
    }
    return length;
  }

  // Get the next node in the list.
  IntrusiveListNode* GetNext() const { return next_; }

  // Get the previous node in the list.
  IntrusiveListNode* GetPrevious() const { return prev_; }

  // If PINDROP_INTRUSIVE_LIST_VALIDATE is 1 perform a very rough validation
  // of all nodes in the list.
  bool ValidateList() const {
#if PINDROP_INTRUSIVE_LIST_VALIDATE
    if (magic_ != k_magic) return false;
    const IntrusiveListNode* const terminator = GetTerminator();
    for (IntrusiveListNode* node = GetNext(); node != terminator;
         node = node->GetNext()) {
      if (node->magic_ != k_magic) return false;
    }
#endif  // PINDROP_INTRUSIVE_LIST_VALIDATE
    return true;
  }

  // Determine whether the specified node is present in this list.
  bool FindNodeInList(IntrusiveListNode* const node_to_find) const {
    const IntrusiveListNode* const terminator = GetTerminator();
    for (IntrusiveListNode* node = GetNext(); node != terminator;
         node = node->GetNext()) {
      if (node_to_find == node) return true;
    }
    return false;
  }

  // Initialize the list node.
  void Initialize() {
    next_ = this;
    prev_ = this;
  }

 private:
#if PINDROP_INTRUSIVE_LIST_VALIDATE
  std::uint32_t magic_;
#endif  // PINDROP_INTRUSIVE_LIST_VALIDATE
  // The next node in the list.
  IntrusiveListNode* prev_;
  // The previous node in the list.
  IntrusiveListNode* next_;

#if PINDROP_INTRUSIVE_LIST_VALIDATE
  static const std::uint32_t k_magic = 0x7157ac01;
#endif  // PINDROP_INTRUSIVE_LIST_VALIDATE
};

// Declares a member function to retrieve a pointer to NodeMemberName.
#define PINDROP_INTRUSIVE_GET_NODE_ACCESSOR(NodeMemberName, FunctionName) \
  IntrusiveListNode* FunctionName() { return &NodeMemberName; }           \
  const IntrusiveListNode* FunctionName() const { return &NodeMemberName; }

// Declares a member function GetListNode() to retrieve a pointer to
// NodeMemberName.
#define PINDROP_INTRUSIVE_GET_NODE(NodeMemberName) \
  PINDROP_INTRUSIVE_GET_NODE_ACCESSOR(NodeMemberName, GetListNode)

// Declares the member function FunctionName of Class to retrieve a pointer
// to a Class instance from a list node pointer.  NodeMemberName references
// the name of the IntrusiveListNode member of Class.
#define PINDROP_INTRUSIVE_LIST_NODE_GET_CLASS_ACCESSOR(Class, NodeMemberName, \
                                                       FunctionName)          \
  static Class* FunctionName(IntrusiveListNode* node) {                       \
    Class* cls = nullptr;                                                     \
    /* This effectively performs offsetof(Class, NodeMemberName) */           \
    /* which ends up in the undefined behavior realm of C++ but in */         \
    /* practice this works with most compilers. */                            \
    return reinterpret_cast<Class*>(                                          \
        reinterpret_cast<std::uint8_t*>(node) -                               \
        reinterpret_cast<std::uint8_t*>(&cls->NodeMemberName));               \
  }                                                                           \
                                                                              \
  static const Class* FunctionName(const IntrusiveListNode* node) {           \
    return FunctionName(const_cast<IntrusiveListNode*>(node));                \
  }

// Declares the member function GetInstanceFromListNode() of Class to retrieve
// a pointer to a Class instance from a list node pointer.  NodeMemberName
// reference the name of the IntrusiveListNode member of Class.
#define PINDROP_INTRUSIVE_LIST_NODE_GET_CLASS(Class, NodeMemberName)    \
  PINDROP_INTRUSIVE_LIST_NODE_GET_CLASS_ACCESSOR(Class, NodeMemberName, \
                                                 GetInstanceFromListNode)

// Declares the macro to iterate over a typed intrusive list.
#define PINDROP_INTRUSIVE_LIST_NODE_ITERATOR(type, listptr, iteidentifier_, \
                                             statement)                     \
  {                                                                         \
    type* terminator = (listptr).GetTerminator();                           \
    for (type* iteidentifier_ = (listptr).GetNext();                        \
         iteidentifier_ != terminator;                                      \
         iteidentifier_ = iteidentifier_->GetNext()) {                      \
      statement;                                                            \
    }                                                                       \
  }

// TypedIntrusiveListNode which supports inserting an object into a single
// doubly linked list.  For objects that need to be inserted in multiple
// doubly linked lists, use IntrusiveListNode.
//
// For example:
//
// class IntegerItem : public TypedIntrusiveListNode<IntegerItem> {
//  public:
//   IntegerItem(std::int32_t value) : value_(value) {}
//   ~IntegerItem() { }
//   std::int32_t GetValue() const { return value_; }
//  private:
//   std::int32_t value_;
// };
//
// int main(int argc, const char *arvg[]) {
//   TypedIntrusiveListNode<IntegerItem> list;
//   IntegerItem a(1);
//   IntegerItem b(2);
//   IntegerItem c(3);
//   list.InsertBefore(&a);
//   list.InsertBefore(&b);
//   list.InsertBefore(&c);
//   for (IntegerItem* item = list.GetNext();
//        item != list.GetTerminator(); item = item->GetNext()) {
//     printf("%d\n", item->GetValue());
//   }
// }
template <typename T>
class TypedIntrusiveListNode {
 public:
  TypedIntrusiveListNode() {}
  ~TypedIntrusiveListNode() {}

  // Insert this object after the specified object.
  void InsertAfter(T* const obj) {
    assert(obj);
    GetListNode()->InsertAfter(obj->GetListNode());
  }

  // Insert this object before the specified object.
  void InsertBefore(T* const obj) {
    assert(obj);
    GetListNode()->InsertBefore(obj->GetListNode());
  }

  // Get the next object in the list.
  // Check against GetTerminator() before deferencing the object.
  T* GetNext() const {
    return GetInstanceFromListNode(GetListNode()->GetNext());
  }

  // Get the previous object in the list.
  // Check against GetTerminator() before deferencing the object.
  T* GetPrevious() const {
    return GetInstanceFromListNode(GetListNode()->GetPrevious());
  }

  // Get the terminator of the list.
  // This should not be dereferenced as it is a pointer to
  // TypedIntrusiveListNode<T> *not* T.
  T* GetTerminator() const {
    return GetInstanceFromListNode(
        const_cast<IntrusiveListNode*>(GetListNode()));
  }

  // Remove this object from the list it's currently in.
  T* Remove() {
    GetListNode()->Remove();
    return GetInstanceFromListNode(GetListNode());
  }

  // Determine whether this object is in a list.
  bool InList() const { return GetListNode()->InList(); }

  // Determine whether this list is empty.
  bool IsEmpty() const { return GetListNode()->IsEmpty(); }

  // Calculate the length of the list.
  std::uint32_t GetLength() const { return GetListNode()->GetLength(); }

  PINDROP_INTRUSIVE_GET_NODE(node_);

  // Get a pointer to the instance of T that contains "node".
  static T* GetInstanceFromListNode(IntrusiveListNode* const node) {
    assert(node);
    // Calculate the pointer to T from the offset.
    return (T*)((std::uint8_t*)node - GetNodeOffset(node));
  }

 private:
  // Node within an intrusive list.
  IntrusiveListNode node_;

  // Get the offset of node_ within this class.
  static std::int32_t GetNodeOffset(IntrusiveListNode* const node) {
    assert(node);
    // Perform some type punning to calculate the offset of node_ in T.
    // WARNING: This could result in undefined behavior with some C++
    // compilers.
    T* obj = (T*)node;
    std::int32_t node_offset =
        (std::int32_t)((std::uint8_t*)(&obj->node_) - (std::uint8_t*)(obj));
    return node_offset;
  }
};

}  // namespace pindrop

#endif  // PINDROP_INTRUSIVE_LIST_H_

