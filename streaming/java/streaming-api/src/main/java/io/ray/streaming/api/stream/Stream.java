package io.ray.streaming.api.stream;

import com.google.common.base.Preconditions;
import io.ray.streaming.api.Language;
import io.ray.streaming.api.context.StreamingContext;
import io.ray.streaming.api.partition.Partition;
import io.ray.streaming.api.partition.impl.RoundRobinPartition;
import io.ray.streaming.operator.Operator;
import io.ray.streaming.operator.StreamOperator;
import io.ray.streaming.python.PythonPartition;
import java.io.Serializable;
import java.util.HashMap;
import java.util.Map;

/**
 * Abstract base class of all stream types.
 *
 * @param <S> Type of stream class
 * @param <T> Type of the data in the stream.
 */
public abstract class Stream<S extends Stream<S, T>, T>
    implements Serializable {
  private final int id;
  private final StreamingContext streamingContext;
  private final Stream inputStream;
  private StreamOperator operator;
  private int parallelism = 1;
  private Map<String, String> config = new HashMap<>();
  private Partition<T> partition;
  private Stream originalStream;

  public Stream(StreamingContext streamingContext, StreamOperator streamOperator) {
    this(streamingContext, null, streamOperator,
         selectPartition(streamOperator));
  }

  public Stream(StreamingContext streamingContext,
                StreamOperator streamOperator,
                Partition<T> partition) {
    this(streamingContext, null, streamOperator, partition);
  }

  public Stream(Stream inputStream, StreamOperator streamOperator) {
    this(inputStream.getStreamingContext(), inputStream, streamOperator,
         selectPartition(streamOperator));
  }

  public Stream(Stream inputStream, StreamOperator streamOperator, Partition<T> partition) {
    this(inputStream.getStreamingContext(), inputStream, streamOperator, partition);
  }

  protected Stream(StreamingContext streamingContext,
                Stream inputStream,
                StreamOperator streamOperator,
                Partition<T> partition) {
    this.streamingContext = streamingContext;
    this.inputStream = inputStream;
    this.operator = streamOperator;
    this.partition = partition;
    this.id = streamingContext.generateId();
    if (inputStream != null) {
      this.parallelism = inputStream.getParallelism();
    }
  }

  /**
   * Create a proxy stream of original stream.
   * Changes in new stream will be reflected in original stream and vice versa
   */
  protected Stream(Stream originalStream) {
    this.originalStream = originalStream;
    this.id = originalStream.getId();
    this.streamingContext = originalStream.getStreamingContext();
    this.inputStream = originalStream.getInputStream();
    this.operator = originalStream.getOperator();
    Preconditions.checkNotNull(operator);
  }

  @SuppressWarnings("unchecked")
  private static <T> Partition<T> selectPartition(Operator operator) {
    switch (operator.getLanguage()) {
      case PYTHON:
        return (Partition<T>) PythonPartition.RoundRobinPartition;
      case JAVA:
        return new RoundRobinPartition<>();
      default:
        throw new UnsupportedOperationException(
            "Unsupported language " + operator.getLanguage());
    }
  }

  public int getId() {
    return id;
  }

  public StreamingContext getStreamingContext() {
    return streamingContext;
  }

  public Stream getInputStream() {
    return inputStream;
  }

  public StreamOperator getOperator() {
    return operator;
  }

  protected void setOperator(StreamOperator operator) {
    this.operator = operator;
    if (isProxyStream()) {
      getOriginalStream().setOperator(operator);
    }
  }

  @SuppressWarnings("unchecked")
  private S self() {
    return (S) this;
  }

  public int getParallelism() {
    return originalStream != null ? originalStream.getParallelism() : parallelism;
  }

  public S setParallelism(int parallelism) {
    if (originalStream != null) {
      originalStream.setParallelism(parallelism);
    } else {
      this.parallelism = parallelism;
    }
    return self();
  }

  @SuppressWarnings("unchecked")
  public Partition<T> getPartition() {
    return originalStream != null ? originalStream.getPartition() : partition;
  }

  @SuppressWarnings("unchecked")
  protected S setPartition(Partition<T> partition) {
    if (originalStream != null) {
      originalStream.setPartition(partition);
    } else {
      this.partition = partition;
    }
    return self();
  }

  public S withConfig(Map<String, String> config) {
    config.forEach(this::withConfig);
    return self();
  }

  public S withConfig(String key, String value) {
    if (isProxyStream()) {
      originalStream.withConfig(key, value);
    } else {
      this.config.put(key, value);
    }
    return self();
  }

  @SuppressWarnings("unchecked")
  public Map<String, String> getConfig() {
    return isProxyStream() ? originalStream.getConfig() : config;
  }

  public boolean isProxyStream() {
    return originalStream != null;
  }

  public Stream getOriginalStream() {
    Preconditions.checkArgument(isProxyStream());
    return originalStream;
  }

  public abstract Language getLanguage();
}
