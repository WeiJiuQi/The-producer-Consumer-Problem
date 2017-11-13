#include <cstdlib>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>
#include <windows.h>
using namespace std;

static const int bufferSize = 10;
static const int kItemsToConsume = 50;

struct ItemBuffer {
	int buffer[bufferSize];
	int read_position;
	int write_position;
	int buffer_item_counter;
	int produced_item_counter;
	int consumed_item_counter;
	mutex mtx;
	condition_variable buffer_full;
	condition_variable buffer_empty;
} itemBuffer;

void Producer()
{
	bool ready_to_exit = false;
	while (1) {
		Sleep(300);
		unique_lock<mutex> lock(itemBuffer.mtx);
		if (itemBuffer.consumed_item_counter < kItemsToConsume) {
			while (itemBuffer.buffer_item_counter == bufferSize) {
				cout << "缓冲区已满，生产者正在等待一个空位置" << endl;
				(itemBuffer.buffer_full).wait(lock);
			}

			itemBuffer.produced_item_counter++;
			(itemBuffer.buffer)[itemBuffer.write_position] = itemBuffer.produced_item_counter;
			itemBuffer.write_position++;
			itemBuffer.buffer_item_counter++;

			if (itemBuffer.write_position == bufferSize)
				itemBuffer.write_position = 0;

			(itemBuffer.buffer_empty).notify_all();
			cout << "生产者线程 " << this_thread::get_id()
				<< " 正在生产第 " << itemBuffer.produced_item_counter
				<< "个项目" << endl;
		}
		else ready_to_exit = true;
		lock.unlock();
		if (ready_to_exit == true) break;
	}
	cout << "生产者线程 " << this_thread::get_id()
		<< " 正在退出" << endl;
}

void Consumer()
{
	bool ready_to_exit = false;
	while (1) {
		Sleep(300);
		unique_lock<mutex> lock(itemBuffer.mtx);
		if (itemBuffer.consumed_item_counter < kItemsToConsume) {
			int item;
			while (itemBuffer.buffer_item_counter == 0) {
				cout << "缓冲区已空，消费者正在等待一个项目" << endl;
				(itemBuffer.buffer_empty).wait(lock);
			}

			item = (itemBuffer.buffer)[itemBuffer.read_position];
			itemBuffer.read_position++;
			itemBuffer.buffer_item_counter--;

			if (itemBuffer.read_position == bufferSize)
				itemBuffer.read_position = 0;

			(itemBuffer.buffer_full).notify_all();
			itemBuffer.consumed_item_counter++;
			cout << "消费者线程 " << this_thread::get_id()
				<< " 正在消费第 " << item << "个项目" << endl;
		}
		else ready_to_exit = true;
		lock.unlock();
		if (ready_to_exit == true) break;
	}
	cout << "消费者线程 " << this_thread::get_id()
		<< " 正在退出" << endl;
}

void InitItemBuffer(ItemBuffer *ir)
{
	ir->write_position = 0;
	ir->read_position = 0;
	ir->buffer_item_counter = 0;
	ir->produced_item_counter = 0;
	ir->consumed_item_counter = 0;
}

int main()
{
	InitItemBuffer(&itemBuffer);
	thread producer1(Producer);
	thread producer2(Producer);
	thread producer3(Producer);

	thread consumer1(Consumer);
	thread consumer2(Consumer);

	producer1.join();
	producer2.join();
	producer3.join();

	consumer1.join();
	consumer2.join();
	system("pause");
}