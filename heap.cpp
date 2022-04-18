#include "heap.h"

void Heap::Insert(const Element& element) {
    elements_.push_back(element);
    PushUp();
}

const Heap::Element& Heap::GetMin() const {
    return elements_[0];
}

void Heap::ExtractMin() {
    std::swap(elements_[0], elements_.back());
    elements_.pop_back();
    PushDown();
}

size_t Heap::Size() const {
    return elements_.size();
}

bool Heap::IsEmpty() const {
    return Size() == 0;
}

void Heap::PushDown() {
    size_t cure_vertex = 0;
    while (LeftSon(cure_vertex) < elements_.size()) {
        size_t min_son = LeftSon(cure_vertex);
        if (RightSon(cure_vertex) < elements_.size() && elements_[min_son].count > elements_[RightSon(cure_vertex)].count) {
            min_son = RightSon(cure_vertex);
        }
        if (elements_[min_son].count >= elements_[cure_vertex].count) {
            break;
        }
        std::swap(elements_[min_son], elements_[cure_vertex]);
        cure_vertex = min_son;
    }
}

void Heap::PushUp() {
    size_t cure_vertex = elements_.size() - 1;
    while (cure_vertex != 0 && elements_[cure_vertex].count < elements_[Parent(cure_vertex)].count) {
        std::swap(elements_[cure_vertex], elements_[Parent(cure_vertex)]);
        cure_vertex = Parent(cure_vertex);
    }
}

size_t Heap::Parent(size_t vertex) {
    return (vertex - 1) / 2;
}

size_t Heap::LeftSon(size_t vertex) {
    return vertex * 2 + 1;
}

size_t Heap::RightSon(size_t vertex) {
    return vertex * 2 + 2;
}