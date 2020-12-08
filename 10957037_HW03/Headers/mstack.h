#ifndef MSTACK_H
#define MSTACK_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

class StackArray {
private:
	int index;
	int capacity;
	glm::mat4* stack;

	void doubleCapacity() {
		capacity *= 2;

		glm::mat4* newStack = new glm::mat4[capacity];
		for (int i = 0; i < capacity / 2; i++) {
			newStack[i] = stack[i];
		}

		delete[] stack;
		stack = newStack;
	};

public:

	StackArray() : index(0), capacity(32) {
		stack = new glm::mat4[capacity];
		stack[0] = glm::mat4(1.0f);
	}

	void push() {
		if (index == capacity - 1) {
			doubleCapacity();
		}
		stack[index + 1] = stack[index];
		index++;
	}

	void pop() {
		if (isEmpty()) {
			std::cout << "Stack is empty. \n";
			return;
		}
		index--;
	}

	bool isEmpty() {
		return (index == 0);
	}

	glm::mat4 top() {
		return stack[index];
	}

	int getSize() {
		return index + 1;
	}

	void save(glm::mat4 matrix) {
		stack[index] = matrix;
	}
};

#endif // !MSTACK_H