#include <thread>  // NOLINT

#include "caffe2/utils/simple_queue.h"
#include "gtest/gtest.h"

namespace caffe2 {

static std::unique_ptr<SimpleQueue<int> > gQueue;

static void ConsumerFunction(int thread_idx) {
  int value;
  while (true) {
    if (!gQueue->Pop(&value)) return;
    CAFFE_LOG_INFO << "Emitting " << value << " from thread " << thread_idx;
  }
}

static void ProducerFunction(int thread_idx, int start, int count) {
  for (int i = 0; i < count; ++i) {
    CAFFE_LOG_INFO << "Pushing " << i + start << " from thread " << thread_idx;
    gQueue->Push(i + start);
  }
}


TEST(SimpleQueueTest, SingleProducerSingleConsumer) {
  gQueue.reset(new SimpleQueue<int>());
  std::thread consumer(ConsumerFunction, 0);
  for (int i = 0; i < 10; ++i) {
    gQueue->Push(i);
  }
  gQueue->NoMoreJobs();
  consumer.join();
}

TEST(SimpleQueueTest, SingleProducerDoubleConsumer) {
  gQueue.reset(new SimpleQueue<int>());
  std::thread consumer0(ConsumerFunction, 0);
  std::thread consumer1(ConsumerFunction, 1);
  for (int i = 0; i < 10; ++i) {
    gQueue->Push(i);
  }
  gQueue->NoMoreJobs();
  consumer0.join();
  consumer1.join();
}


TEST(SimpleQueueTest, DoubleProducerDoubleConsumer) {
  gQueue.reset(new SimpleQueue<int>());
  std::thread producer0(ProducerFunction, 0, 0, 10);
  std::thread producer1(ProducerFunction, 0, 10, 10);
  std::thread consumer0(ConsumerFunction, 2);
  std::thread consumer1(ConsumerFunction, 3);
  producer0.join();
  producer1.join();
  gQueue->NoMoreJobs();
  consumer0.join();
  consumer1.join();
}

TEST(SimpleQueueDeathTest, CannotAddAfterQueueFinished) {
  gQueue.reset(new SimpleQueue<int>());
  gQueue->Push(0);
  gQueue->NoMoreJobs();
  EXPECT_DEATH(gQueue->Push(0),
               "Check failed: !no_more_jobs_ Cannot push to a closed queue.");
}


}  // namespace caffe2


