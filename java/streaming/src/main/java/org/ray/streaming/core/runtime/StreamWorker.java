package org.ray.streaming.core.runtime;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.List;
import org.ray.api.annotation.RayRemote;
import org.ray.streaming.api.collector.Collector;
import org.ray.streaming.core.command.BatchInfo;
import org.ray.streaming.core.graph.ExecutionEdge;
import org.ray.streaming.core.graph.ExecutionGraph;
import org.ray.streaming.core.graph.ExecutionNode;
import org.ray.streaming.core.graph.ExecutionNode.NodeType;
import org.ray.streaming.core.graph.ExecutionTask;
import org.ray.streaming.core.processor.MasterProcessor;
import org.ray.streaming.core.processor.OneInputProcessor;
import org.ray.streaming.core.processor.SourceProcessor;
import org.ray.streaming.core.processor.StreamProcessor;
import org.ray.streaming.core.runtime.collector.RayCallCollector;
import org.ray.streaming.core.runtime.context.RayRuntimeContext;
import org.ray.streaming.core.runtime.context.RuntimeContext;
import org.ray.streaming.core.runtime.context.WorkerContext;
import org.ray.streaming.core.tasks.OneInputStreamTask;
import org.ray.streaming.core.tasks.SourceStreamTask;
import org.ray.streaming.core.tasks.StreamTask;
import org.ray.streaming.io.reader.RecordReader;
import org.ray.streaming.io.writer.RecordWriter;
import org.ray.streaming.message.Message;
import org.ray.streaming.message.Record;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * The stream worker, it is a ray actor.
 */
@RayRemote
public class StreamWorker implements Serializable {

  private static final Logger LOGGER = LoggerFactory.getLogger(StreamWorker.class);

  private int taskId;
  private WorkerContext workerContext;
  private StreamProcessor streamProcessor;
  private NodeType nodeType;
  private StreamTask task;
  private Thread taskThread;

  public StreamWorker() {
  }

  public Boolean init(WorkerContext workerContext) {
    this.workerContext = workerContext;
    this.taskId = workerContext.getTaskId();
    ExecutionGraph executionGraph = this.workerContext.getExecutionGraph();
    ExecutionTask executionTask = executionGraph.getExecutionTaskByTaskId(taskId);
    ExecutionNode executionNode = executionGraph.getExecutionNodeByTaskId(taskId);

    this.nodeType = executionNode.getNodeType();
    this.streamProcessor = executionNode.getStreamProcessor();
    LOGGER.debug("Initializing StreamWorker, taskId: {}, operator: {}.", taskId, streamProcessor);

    // create record writer
    List<ExecutionEdge> executionEdges = executionNode.getExecutionEdgeList();
    List<Collector> collectors = new ArrayList<>();
    for (ExecutionEdge executionEdge : executionEdges) {
      collectors.add(new RecordWriter(taskId, executionEdge, executionGraph));
    }

    // create record reader
    List<ExecutionEdge> inputEdges = executionNode.getInputsEdges();
    RecordReader recordReader = null;
    for (ExecutionEdge executionEdge : inputEdges) {
      recordReader = new RecordReader(taskId, executionEdge, executionGraph);
    }

    RuntimeContext runtimeContext = new RayRuntimeContext(executionTask, workerContext.getConfig(),
        executionNode.getParallelism());
    if (this.nodeType == NodeType.MASTER) {
      ((MasterProcessor) streamProcessor).open(collectors, runtimeContext, executionGraph);
    } else {
      this.streamProcessor.open(recordReader, collectors, runtimeContext);
    }

    if (streamProcessor instanceof OneInputProcessor) {
      task = new OneInputStreamTask((OneInputProcessor) streamProcessor);
    } else if (streamProcessor instanceof SourceProcessor) {
      task = new SourceStreamTask((SourceProcessor) streamProcessor);
    } else if (streamProcessor instanceof MasterProcessor) {
      return true;
    } else {
      throw new RuntimeException("Unsupport type: " + streamProcessor);
    }

    taskThread = new Thread(task, "StreamTask");
    taskThread.start();
    return true;
  }

  public Boolean process(Message message) {
    LOGGER.debug("Processing message, taskId: {}, message: {}.", taskId, message);
    if (nodeType == NodeType.SOURCE) {
      Record record = message.getRecord(0);
      BatchInfo batchInfo = (BatchInfo) record.getValue();
      this.streamProcessor.process(batchInfo.getBatchId());
    } else {
      List<Record> records = message.getRecordList();
      for (Record record : records) {
        record.setBatchId(message.getBatchId());
        record.setStream(message.getStream());
        this.streamProcessor.process(record);
      }
    }
    return true;
  }
}
