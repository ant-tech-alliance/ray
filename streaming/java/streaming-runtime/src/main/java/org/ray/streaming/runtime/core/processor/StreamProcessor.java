package org.ray.streaming.runtime.core.processor;

import java.util.List;

import org.ray.streaming.operator.Operator;
import org.ray.streaming.operator.OperatorType;
import org.slf4j.Logger;

import org.ray.streaming.api.collector.Collector;
import org.ray.streaming.api.context.RuntimeContext;
import org.ray.streaming.runtime.util.LoggerFactory;

/**
 * StreamingProcessor is a process unit for a operator.
 *
 * @param <T> The type of process data.
 * @param <P> Type of the specific operator class.
 */
public abstract class StreamProcessor<T, P extends Operator> implements Processor<T> {

  private static final Logger LOG = LoggerFactory.getLogger(StreamProcessor.class);

  protected List<Collector> collectors;
  protected RuntimeContext runtimeContext;
  protected P operator;

  public StreamProcessor(P operator) {
    this.operator = operator;
  }

  @Override
  public void open(List<Collector> collectors, RuntimeContext runtimeContext) {
    this.collectors = collectors;
    this.runtimeContext = runtimeContext;
    if (operator != null) {
      this.operator.open(collectors, runtimeContext);
    }
    LOG.info("opened {}", this);
  }

  public OperatorType getOperatorType() {
    return operator.getOpType();
  }

  @Override
  public String toString() {
    return this.getClass().getSimpleName();
  }
}
