package org.ray.streaming.runtime.worker.context;

import java.nio.ByteBuffer;
import java.util.Collection;

import org.ray.streaming.message.Record;

import org.ray.streaming.api.collector.Collector;
import org.ray.streaming.api.partition.Partition;
import org.ray.streaming.runtime.core.transfer.ChannelID;
import org.ray.streaming.runtime.core.transfer.DataWriter;
import org.ray.streaming.runtime.util.Serializer;

/**
 */
public class StreamCollector implements Collector<Record> {

  private DataWriter dataWriter;
  private Partition partition;
  private ChannelID[] outputQueue;

  public StreamCollector(Collection<String> outputQueueIds, DataWriter dataWriter, Partition partition) {
    this.outputQueue = outputQueueIds.stream().map(ChannelID :: from).toArray(ChannelID[]::new);
    this.dataWriter = dataWriter;
    this.partition = partition;
  }

  @Override
  public void collect(Record value) {
    int[] partitions = this.partition.partition(value, outputQueue.length);
    ByteBuffer messageBuffer = ByteBuffer.wrap(Serializer.encode(value));
    for (int partition : partitions) {
      dataWriter.write(outputQueue[partition], messageBuffer);
    }
  }
}
