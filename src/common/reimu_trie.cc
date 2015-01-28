//
// The MIT License (MIT)
//
// Copyright 2013-2014 The MilkCat Project Developers
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// reimu_trie.cc --- Created at 2014-10-16
// Reimu x Marisa :P
//

#include "common/reimu_trie.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

#define _assert(x) assert(x)
#define XOR(a, b) ((a) ^ (b))
#define CALC_BASE_FROM_TO_AND_LABEL(to, label) ((to) ^ (label))
#define CALC_LABEL_FROM_BASE_AND_TO(base, to) ((base) ^ (to))
#define NODE_INDEX_TO_BLOCK_INDEX(node_idx) ((node_idx) >> 8)
#define BLOCK_INDEX_TO_FIRST_NODE_INDEX(node_idx) ((node_idx) << 8)
#define CLOSED_THRESHOLD 1

namespace milkcat {

class ReimuTrie::Impl {
 public:
  Impl();
  ~Impl();

  // These functions are same to functions in ReimuTrie::
  static Impl *Open(const char *filename);
  int32 Get(const char *key, int32 default_value) const;
  void Put(const char *key, int32 value);
  bool Save(const char *filename);
  int size() const;
  bool Check();
  void SetArray(void *array);
  void *array() const { return reinterpret_cast<void *>(array_); }
  bool Traverse(
      int *from, const char *key, int32 *value, int32 default_value) const;
 private:
  class Node;
  class Block;

  enum {
    kBlockLinkListEnd = -1,
    kBaseNone = -1
  };

  // To create the first block
  void Initialize();

  // Internal functions for `Check`
  int CheckBlock(int block_idx);
  bool CheckList(int head, std::vector<bool> *block_bitmap);

  // Returns the next index from `from` with character ch
  int Next(int from, uint8 label);

  // Removes an empty from block
  void PopEmptyNode(int node_idx);

  // Add the node back to the empty list of block
  void PushEmptyNode(int node_idx);

  // Transfers a block to another block link list
  void TransferBlock(int block_idx, int *from_list, int *to_list);

  // Returns an empty node
  int FindEmptyNode();

  // Resolves conflict
  int ResolveConflict(int *from, int base, uint8 label);

  // Count the number of children for a base node
  int ChildrenCount(int from_idx, int base_idx);

  // Add a new block and its nodes into the trie
  int AddBlock();

  // Find a empty place that contains each child in `child`. Returns the base
  // node index
  int FindEmptyRange(uint8 *child, int count);

  // Enumerates each child in the sub tree of base_idx, stores them into child
  // and returns the child number
  int EnumerateChild(int from_idx, int base_idx, uint8 *child);

  // Moves each child (in `child`) of `base` into `new_base`
  void MoveSubTree(int from, int base, int new_base, uint8 *child, 
                   int child_count);

  // Dumps the values in block, just for debugging
  void DumpBlock(int block_idx);

  // Restores the block data for `Put`
  void Restore();

  Node *array_;
  Block *block_;
  int size_;
  int capacity_;

  int open_block_head_;
  int closed_block_head_;
  int full_block_head_;
  bool use_external_array_;
};

// Stores block data. A block is a sequence of 256 nodes
class ReimuTrie::Impl::Block {
 public:
  Block();

  // `previous` field, previous node in the chain
  void set_previous(int previous) { previous_ = previous; }
  int previous() const { return previous_; };

  // `next` field, next node in the chain
  void set_next(int next) { next_ = next; }
  int next() const { return next_; };

  // `empty_head` field, first empty node of the block
  void set_empty_head(int empty_head) { empty_head_ = empty_head; }
  int empty_head() const { return empty_head_; }

  // `empty_number` field
  void set_empty_number(int empty_number) { empty_number_ = empty_number; }
  int empty_number() const { return empty_number_; };

 private:
  int previous_;
  int next_;
  int empty_head_;
  int empty_number_;
};

// A (base, check) pair of double array trie
class ReimuTrie::Impl::Node {
 public:
  // `base` related functions
  void set_base(int32 base) { base_ = base; };
  int32 base() const {
    _assert(base_ >= kBaseNone);
    return base_;
  }
  void set_previous(int32 previous) { base_ = -previous; }
  int32 previous() const {
    _assert(base_ < 0);
    return -base_;
  }
  void set_value(int32 value) { base_ = value; }
  int32 value() const { 
    _assert(check_ >= 0);
    return base_;
  }
  // `check` related functions
  void set_check(int32 check) { check_ = check; }
  int32 check() const { return check_; }
  void set_next(int32 next) { check_ = -next; }
  int32 next() const { 
    _assert(check_ < 0);
    return -check_; 
  }
  // Returns true if current node is empty
  bool empty() const { return check_ < 0; }

  int32 base_;
  int32 check_;
};

ReimuTrie::ReimuTrie() { impl_ = new ReimuTrie::Impl(); }
ReimuTrie::~ReimuTrie() { delete impl_; }
ReimuTrie::int32 ReimuTrie::Get(const char *key, int32 default_value) const {
  return impl_->Get(key, default_value);
}
void ReimuTrie::Put(const char *key, int32 value) { impl_->Put(key, value); }
ReimuTrie *ReimuTrie::Open(const char *filename) {
  Impl *impl = Impl::Open(filename);
  if (impl != NULL) {
    ReimuTrie *self = new ReimuTrie();
    self->impl_ = impl;
    return self;
  } else {
    return NULL;
  }
}
bool ReimuTrie::Save(const char *filename) { return impl_->Save(filename); }
int ReimuTrie::size() const { return impl_->size(); }
void ReimuTrie::_Check() { impl_->Check(); }
void ReimuTrie::SetArray(void *array) { impl_->SetArray(array); }
bool ReimuTrie::Traverse(
      int *from, const char *key, int32 *value, int32 default_value) const {
  return impl_->Traverse(from, key, value, default_value);
}
void *ReimuTrie::array() const { return impl_->array(); }

ReimuTrie::Impl::Block::Block(): previous_(0),
                                 next_(0),
                                 empty_head_(0),
                                 empty_number_(256) {
}

ReimuTrie::Impl::Impl(): array_(NULL),
                         block_(NULL),
                         size_(0),
                         capacity_(0),
                         open_block_head_(kBlockLinkListEnd),
                         closed_block_head_(kBlockLinkListEnd),
                         full_block_head_(kBlockLinkListEnd),
                         use_external_array_(false) {
}
ReimuTrie::Impl::~Impl() {
  if (use_external_array_ == false) free(array_);
  array_ = NULL;

  free(block_);
  block_ = NULL;
}

int ReimuTrie::Impl::size() const {
  return size_ * sizeof(Node);
}

void ReimuTrie::Impl::Initialize() {
  size_ = 256;
  capacity_ = 256;
  array_ = reinterpret_cast<Node *>(malloc(size_ * sizeof(Node)));
  block_ = reinterpret_cast<Block *>(
        malloc(NODE_INDEX_TO_BLOCK_INDEX(size_) * sizeof(Block)));
  array_[0].set_base(0);
  array_[0].set_check(-1);

  array_[1].set_previous(255);
  array_[1].set_next(2);
  for (int i = 2; i < 255; ++i) {
    array_[i].set_previous(i - 1);
    array_[i].set_next(i + 1);
  }
  array_[255].set_next(1);
  array_[255].set_previous(254);

  block_[0] = Block();
  block_[0].set_empty_number(255);
  block_[0].set_empty_head(1);
}

void ReimuTrie::Impl::Restore() {
  int block_num = NODE_INDEX_TO_BLOCK_INDEX(size_);

  block_ = reinterpret_cast<Block *>(malloc(block_num * sizeof(Block)));
  for (int block_idx = 0; block_idx < block_num; ++block_idx) {
    block_[block_idx] = Block();
    // `empty_head` and `empty_number` field
    int first_node = BLOCK_INDEX_TO_FIRST_NODE_INDEX(block_idx);
    int empty = 0;
    for (int node_idx = first_node; node_idx < first_node + 256; ++node_idx) {
      if (array_[node_idx].empty() && node_idx != 0) {
        if (empty == 0) block_[block_idx].set_empty_head(node_idx);
        empty++;
      }
    }
    block_[block_idx].set_empty_number(empty);
    // Put into block linklist
    if (empty == 0) {
      TransferBlock(block_idx, NULL, &full_block_head_);
    } else if (empty <= CLOSED_THRESHOLD) {
      TransferBlock(block_idx, NULL, &closed_block_head_);
    } else {
      TransferBlock(block_idx, NULL, &open_block_head_);
    }
  }
}

bool ReimuTrie::Impl::Save(const char *filename) {
  FILE *fd = fopen(filename, "wb");
  if (fd == NULL) return false;

  int write_size = fwrite(array_, sizeof(Node), size_, fd);
  fclose(fd);

  if (write_size == size_) {
    return true;
  } else {
    return false;
  }
}

ReimuTrie::Impl *ReimuTrie::Impl::Open(const char *filename) {
  FILE *fd = fopen(filename, "rb");
  if (fd == NULL) return NULL;

  Impl *impl = new Impl();

  fseek(fd, 0, SEEK_END);
  impl->size_ = ftell(fd) / sizeof(Node);
  impl->capacity_ = impl->size_;
  impl->array_ = reinterpret_cast<Node *>(malloc(impl->size_ * sizeof(Node)));
  fseek(fd, 0, SEEK_SET);

  int read_size = fread(impl->array_, sizeof(Node), impl->size_, fd);
  fclose(fd);
  if (read_size == impl->size_) {
    return impl;
  } else {
    delete impl;
    return NULL;
  }
}

bool ReimuTrie::Impl::Traverse(
    int *from, const char *key, int32 *value, int32 default_value) const {
  if (array_ == NULL) return false;

  const uint8 *p = reinterpret_cast<const uint8 *>(key);
  int to, base;
  while (*p != 0) {
    base = array_[*from].base();
    to = XOR(base, *p);
    if (array_[to].check() != *from) return false;
    *from = to;
    ++p;
  }
  to = XOR(array_[*from].base(), 0);
  if (array_[to].check() != *from) {
    *value = default_value;
  } else {
    *value = array_[to].value();
  }
  return true;  
}

ReimuTrie::int32
ReimuTrie::Impl::Get(const char *key, int32 default_value) const {
  int from = 0;
  int32 value;
  bool path_exists = Traverse(&from, key, &value, default_value);
  if (path_exists == false) return default_value;
  return value;
}

void ReimuTrie::Impl::Put(const char *key, int32 value) {
  if (use_external_array_ == true) {
    // External array is read only
    return ;
  } else if (array_ == NULL) {
    Initialize();
  } else if (block_ == NULL) {
    Restore();
  }

  const uint8 *p = reinterpret_cast<const uint8 *>(key);
  int from = 0, to;
  while (*p != 0) {
    from = Next(from, *p);
    ++p;
  }
  to = Next(from, 0);

  array_[to].set_value(value);
}

int ReimuTrie::Impl::AddBlock() {
  if (size_ == capacity_) {
    capacity_ += capacity_;
    array_ = reinterpret_cast<Node *>(
        realloc(array_, capacity_ * sizeof(Node)));
    block_ = reinterpret_cast<Block *>(
        realloc(block_, NODE_INDEX_TO_BLOCK_INDEX(capacity_) * sizeof(Block)));
  }

  int block_idx = NODE_INDEX_TO_BLOCK_INDEX(size_);
  block_[block_idx] = Block();
  block_[block_idx].set_empty_head(size_);

  // Build empty node linklist
  array_[size_].set_previous(size_ + 255);
  array_[size_].set_next(size_ + 1);
  for (int i = 1; i < 255; ++i) {
    array_[size_ + i].set_next(size_ + i + 1);
    array_[size_ + i].set_previous(size_ + i - 1);
  }
  array_[size_ + 255].set_previous(size_ + 254);
  array_[size_ + 255].set_next(size_);

  TransferBlock(block_idx, NULL, &open_block_head_);
  size_ += 256;
  return NODE_INDEX_TO_BLOCK_INDEX(size_) - 1;
}

int ReimuTrie::Impl::FindEmptyNode() {
  if (closed_block_head_ != kBlockLinkListEnd) {
    return block_[closed_block_head_].empty_head();
  }
  if (open_block_head_ != kBlockLinkListEnd) {
    return block_[open_block_head_].empty_head();
  }
  return BLOCK_INDEX_TO_FIRST_NODE_INDEX(AddBlock());
}

int ReimuTrie::Impl::FindEmptyRange(uint8 *child, int count) {
  if (open_block_head_ != kBlockLinkListEnd) {
    int block_idx = open_block_head_;
    // Traverse the opened block linklist
    do {
      if (block_[block_idx].empty_number() >= count) {
        // Traverse the nodes in block, find the qualified base node
        int first_node_idx = BLOCK_INDEX_TO_FIRST_NODE_INDEX(block_idx);
        for (int base = first_node_idx; base < first_node_idx + 256; ++base) {
          int i = 0;
          for (; i < count; ++i) {
            // Node - 0 is the special node, always left empty
            if (XOR(base, child[i]) == 0) break;
            if (array_[XOR(base, child[i])].empty() == false) break;
          }
          // `it == labels.end()` indicates an empty range founded
          if (i == count) return base;
        }
      }
      block_idx = block_[block_idx].next();
    } while (block_idx != open_block_head_);
  }
  return BLOCK_INDEX_TO_FIRST_NODE_INDEX(AddBlock());
}

int ReimuTrie::Impl::Next(int from, uint8 label) {
  int base = array_[from].base();
  int to;

  if (kBaseNone == base) {
    // Node `from` have no base value
    to = FindEmptyNode();
    array_[from].set_base(CALC_BASE_FROM_TO_AND_LABEL(to, label));
    PopEmptyNode(to);
    array_[to].set_base(kBaseNone);
    array_[to].set_check(from);
  } else {
    to = XOR(base, label);
    if (array_[to].empty()) {
      // Luckily, `to` is an empty node
      PopEmptyNode(to);
      array_[to].set_base(kBaseNone);
      array_[to].set_check(from);
    } else if (array_[to].check() != from) {
      // Conflict detected!
      int new_base = ResolveConflict(&from, base, label);
      to = XOR(new_base, label);
      PopEmptyNode(to);
      array_[to].set_base(kBaseNone);
      array_[to].set_check(from);
    }
    // Else, `label` child of node `from` already exists, do nothing
  }

  return to;
}

void ReimuTrie::Impl::PopEmptyNode(int node_idx) {
  _assert(array_[node_idx].empty());
  int block_idx = NODE_INDEX_TO_BLOCK_INDEX(node_idx);

  int empty_number = block_[block_idx].empty_number();
  block_[block_idx].set_empty_number(empty_number - 1);
  if (empty_number == 1) {
    // Last node in the block
    TransferBlock(block_idx, &closed_block_head_, &full_block_head_);
  } else {
    Node *node = array_ + node_idx;

    // Remove the node from empty node linklist in block
    _assert(array_[node->previous()].empty());
    array_[node->previous()].set_next(node->next());
    _assert(array_[node->next()].empty());
    array_[node->next()].set_previous(node->previous());
    
    // Change empty head
    if (block_[block_idx].empty_head() == node_idx) {
      block_[block_idx].set_empty_head(node->next());
    }

    // Transfer into closed blocklist when only remians one empty slot
    if (empty_number == 1 + CLOSED_THRESHOLD) {
      TransferBlock(block_idx, &open_block_head_, &closed_block_head_);
    }
  }
}

void ReimuTrie::Impl::PushEmptyNode(int node_idx) {
  int block_idx = NODE_INDEX_TO_BLOCK_INDEX(node_idx);
  Node *node = array_ + node_idx;

  int empty_number = block_[block_idx].empty_number();
  block_[block_idx].set_empty_number(empty_number + 1);
  if (empty_number == 0) {
    // Current block is full
    TransferBlock(block_idx, &full_block_head_, &closed_block_head_);
    node->set_previous(node_idx);
    node->set_next(node_idx);
    block_[block_idx].set_empty_head(node_idx);
  } else {
    // Add the node to the tail of empty linklist
    int empty_head = block_[block_idx].empty_head();
    node->set_next(empty_head);
    node->set_previous(array_[empty_head].previous());
    array_[node->previous()].set_next(node_idx);
    array_[node->next()].set_previous(node_idx);

    if (empty_number == CLOSED_THRESHOLD) {
      TransferBlock(block_idx, &closed_block_head_, &open_block_head_);
    }
  }
}

void ReimuTrie::Impl::TransferBlock(int block_idx,
                                    int *from_list,
                                    int *to_list) {
  // Never transfer block `0`
  if (block_idx == 0) return;

  Block *block = block_ + block_idx;
  if (from_list != NULL) {
    // Transfer out from `from_list`
    if (block->next() == block_idx) {
      _assert(*from_list == block_idx);
      *from_list = kBlockLinkListEnd;
    } else {
      // Remove the block from linklist
      block_[block->previous()].set_next(block->next());
      block_[block->next()].set_previous(block->previous());

      if (*from_list == block_idx) *from_list = block->next();
    }
  }

  // Transfer into `to_list`
  if (kBlockLinkListEnd == *to_list) {
    // `to_list` is NULL
    block->set_previous(block_idx);
    block->set_next(block_idx);
    *to_list = block_idx;
  } else {
    // Add the block into the tail of `to_list`
    int head_idx = *to_list;
    int tail_idx = block_[head_idx].previous();

    block_[head_idx].set_previous(block_idx);
    block->set_next(head_idx);
    block_[tail_idx].set_next(block_idx);
    block->set_previous(tail_idx);
  }
}

int ReimuTrie::Impl::EnumerateChild(int from_idx, int base_idx, uint8 *child) {
  int count = 0;
  for (int label = 0; label < 256; ++label) {
    if (array_[XOR(base_idx, label)].check() == from_idx) {
      child[count] = static_cast<uint8>(label);
      count++;
    }
  }
  return count;
}

void ReimuTrie::Impl::MoveSubTree(int from,
                                  int base,
                                  int new_base,
                                  uint8 *child,
                                  int child_count) {
  for (int i = 0; i < child_count; ++i) {
    uint8 label = child[i];
    int child_idx = XOR(base, label);
    _assert(array_[child_idx].check() == from);
    int new_child_idx = XOR(new_base, label);

    _assert(array_[child_idx].empty() == false);
    _assert(array_[new_child_idx].empty());

    PopEmptyNode(new_child_idx);
    array_[new_child_idx] = array_[child_idx];

    // Change the check value of the children of `array_[child_idx]`
    int child_base = array_[child_idx].base();
    for (int i = 0; i < 256; ++i) {
      int grandson_idx = XOR(child_base, i);
      if (array_[grandson_idx].check() == child_idx) {
        array_[grandson_idx].set_check(new_child_idx);
      }
    }
    PushEmptyNode(child_idx);
  }
}

int ReimuTrie::Impl::ResolveConflict(int *from, int base, uint8 label) {
  int node_idx = XOR(base, label);
  int conflicted_from = array_[node_idx].check();
  int conflicted_base;
  _assert(conflicted_from != node_idx);
  if (conflicted_from == node_idx) {
    // When it is a value node
    conflicted_base = conflicted_from;
  } else {
    conflicted_base = array_[conflicted_from].base();
  }
  _assert(NODE_INDEX_TO_BLOCK_INDEX(conflicted_base) ==
          NODE_INDEX_TO_BLOCK_INDEX(base));

  // Determine which tree to remove
  uint8 child[256];
  uint8 conflicted_child[256];

  int child_count = EnumerateChild(*from, base, child);
  int conflicted_child_count = EnumerateChild(
      conflicted_from,
      conflicted_base,
      conflicted_child);

  int new_base;
  if (child_count + 1 > conflicted_child_count) {
    // Move the conflicted sub tree
    new_base = FindEmptyRange(conflicted_child, conflicted_child_count);
    array_[conflicted_from].set_base(new_base);
    MoveSubTree(conflicted_from,
                conflicted_base,
                new_base,
                conflicted_child,
                conflicted_child_count);

    // When the from node has been moved
    if (array_[*from].empty()) {
      int from_label = CALC_LABEL_FROM_BASE_AND_TO(conflicted_base,
                                                   *from);
      *from = XOR(new_base, from_label);
    }
    return base;
  } else {
    // Move current sub tree
    // Append `label` into `child` to find the empty place including `label`
    child[child_count] = label;
    new_base = FindEmptyRange(child, child_count + 1);
    array_[*from].set_base(new_base);
    MoveSubTree(*from, base, new_base, child, child_count);
    return new_base;
  }
}

bool ReimuTrie::Impl::CheckList(int head, std::vector<bool> *block_bitmap) {
  std::vector<bool> &bitmap = *block_bitmap;
  if (head != kBlockLinkListEnd) {
    int block_idx = head;
    do {
      int empty = CheckBlock(block_idx);
      if (head == open_block_head_) {
        assert(empty > CLOSED_THRESHOLD);
      } else if (head == closed_block_head_) {
        assert(empty > 0 && empty <= CLOSED_THRESHOLD);
      } else if (head == full_block_head_) {
        assert(empty == 0);
      } else {
        assert(false);
      }
      bitmap[block_idx] = true;
      // Checks the `previous` and `next` link between blocks
      assert(block_[block_[block_idx].next()].previous() == block_idx);
      block_idx = block_[block_idx].next();
    } while (block_idx != head);  
  }
  return true;
}

int ReimuTrie::Impl::CheckBlock(int block_idx) {
  int node_idx_start = BLOCK_INDEX_TO_FIRST_NODE_INDEX(block_idx);
  int empty = 0;
  for (int i = node_idx_start; i < node_idx_start + 256; ++i) {
    Node *node = array_ + i;
    // `node_idx_start` == 0 is the special first node
    if (node->check() < 0 && i != 0) {
      empty++;
      assert(node->previous() > 0);

      // Checks the `next` and `previous` between empty nodes
      Node *previous_node = array_ + node->previous();
      Node *next_node = array_ + node->next();
      assert(previous_node->next() == i);
      assert(next_node->previous() == i);
    } else if (node->check() >= 0) {
      assert(node->base() >= kBaseNone);
      if (node->check() != 0) assert(array_[node->check()].empty() == false);
    }
  }
  // Check `empty_number`
  assert(block_[block_idx].empty_number() == empty);
  return empty;
}

bool ReimuTrie::Impl::Check() {
  std::vector<bool> block_bitmap;
  int block_num = NODE_INDEX_TO_BLOCK_INDEX(size_);
  block_bitmap.resize(block_num, false);
  CheckList(open_block_head_, &block_bitmap);
  CheckList(closed_block_head_, &block_bitmap);
  CheckList(full_block_head_, &block_bitmap);

  // Ensures every block is in blocklist except block `0`
  for (int i = 1; i < block_num; ++i) {
    assert(block_bitmap[i]);
  }
  return true;
}

void ReimuTrie::Impl::SetArray(void *array) {
  delete array_;
  array_ = reinterpret_cast<Node *>(array);

  delete block_;
  block_ = NULL;

  size_ = 0;
  capacity_ = 0;
  open_block_head_ = kBlockLinkListEnd;
  closed_block_head_ = kBlockLinkListEnd;
  full_block_head_ = kBlockLinkListEnd;
  use_external_array_ = true;
}

void ReimuTrie::Impl::DumpBlock(int block_idx) {
  printf("BLOCK: %d\n", block_idx);
  printf("node_idx = %d\n", BLOCK_INDEX_TO_FIRST_NODE_INDEX(block_idx));
  printf("empty_number = %d\n", block_[block_idx].empty_number());
  printf("-------\n");
  for (int node_idx = BLOCK_INDEX_TO_FIRST_NODE_INDEX(block_idx);
       node_idx < BLOCK_INDEX_TO_FIRST_NODE_INDEX(block_idx) + 256;
       ++node_idx) {
    printf("%d  %d  %d\n",
           node_idx,
           array_[node_idx].base_,
           array_[node_idx].check_);
  }
  printf("-------\n"); 
}

}  // namespace milkcat
