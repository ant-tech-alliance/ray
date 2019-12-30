package org.ray.streaming.jobgraph;

import java.util.List;

import com.google.common.collect.Lists;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.testng.Assert;
import org.testng.annotations.Test;

import org.ray.streaming.api.context.StreamingContext;
import org.ray.streaming.api.partition.impl.KeyPartition;
import org.ray.streaming.api.partition.impl.RoundRobinPartition;
import org.ray.streaming.api.stream.DataStream;
import org.ray.streaming.api.stream.StreamSink;
import org.ray.streaming.api.stream.StreamSource;

public class JobGraphBuilderTest {

  private static final Logger LOGGER = LoggerFactory.getLogger(JobGraphBuilderTest.class);

  @Test
  public void testDataSync() {
    JobGraph jobGraph = buildDataSyncPlan();
    List<JobVertex> jobVertexList = jobGraph.getJobVertexList();
    List<JobEdge> jobEdgeList = jobGraph.getJobEdgeList();

    Assert.assertEquals(jobVertexList.size(), 2);
    Assert.assertEquals(jobEdgeList.size(), 1);

    JobEdge jobEdge = jobEdgeList.get(0);
    Assert.assertEquals(jobEdge.getPartition().getClass(), RoundRobinPartition.class);

    JobVertex sinkVertex = jobVertexList.get(1);
    JobVertex sourceVertex = jobVertexList.get(0);
    Assert.assertEquals(sinkVertex.getVertexType(), VertexType.SINK);
    Assert.assertEquals(sourceVertex.getVertexType(), VertexType.SOURCE);

  }

  public JobGraph buildDataSyncPlan() {
    StreamingContext streamingContext = StreamingContext.buildContext();
    DataStream<String> dataStream = StreamSource.buildSource(streamingContext,
        Lists.newArrayList("a", "b", "c"));
    StreamSink streamSink = dataStream.sink(x -> LOGGER.info(x));
    JobGraphBuilder jobGraphBuilder = new JobGraphBuilder(Lists.newArrayList(streamSink));

    JobGraph jobGraph = jobGraphBuilder.build();
    return jobGraph;
  }

  @Test
  public void testKeyByPlan() {
    JobGraph jobGraph = buildKeyByPlan();
    List<JobVertex> jobVertexList = jobGraph.getJobVertexList();
    List<JobEdge> jobEdgeList = jobGraph.getJobEdgeList();

    Assert.assertEquals(jobVertexList.size(), 3);
    Assert.assertEquals(jobEdgeList.size(), 2);

    JobVertex source = jobVertexList.get(0);
    JobVertex map = jobVertexList.get(1);
    JobVertex sink = jobVertexList.get(2);

    Assert.assertEquals(source.getVertexType(), VertexType.SOURCE);
    Assert.assertEquals(map.getVertexType(), VertexType.TRANSFORM);
    Assert.assertEquals(sink.getVertexType(), VertexType.SINK);

    JobEdge keyBy2Sink = jobEdgeList.get(0);
    JobEdge source2KeyBy = jobEdgeList.get(1);

    Assert.assertEquals(keyBy2Sink.getPartition().getClass(), KeyPartition.class);
    Assert.assertEquals(source2KeyBy.getPartition().getClass(), RoundRobinPartition.class);
  }

  public JobGraph buildKeyByPlan() {
    StreamingContext streamingContext = StreamingContext.buildContext();
    DataStream<String> dataStream = StreamSource.buildSource(streamingContext,
        Lists.newArrayList("1", "2", "3", "4"));
    StreamSink streamSink = dataStream.keyBy(x -> x)
        .sink(x -> LOGGER.info(x));
    JobGraphBuilder jobGraphBuilder = new JobGraphBuilder(Lists.newArrayList(streamSink));

    JobGraph jobGraph = jobGraphBuilder.build();
    return jobGraph;
  }

}