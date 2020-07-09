package io.ray.runtime.metric;

import com.google.common.base.Preconditions;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;


/**
 * Gauge metric for recording last value and mapping object from stats.
 */
public class Gauge implements Metric {

  private String name;
  private double value;
  private long gaugePtr = 0L;
  Map<TagKey, String> tags;

  public Gauge(String name, String description, String unit, Map<TagKey, String> tags) {
    gaugePtr = registerGaugeNative(name, description, unit,
      tags.keySet().stream().map(TagKey::getTagKey).collect(Collectors.toList()));
    this.name = name;

    this.tags = tags;
  }

  private native long registerGaugeNative(String name, String description,
                                          String unit, List<String> tagKeys);

  private native void unregisterGauge(long gaugePtr);

  public void update(double value) {
    this.value = value;
  }

  public void update(double value, Map<TagKey, String> tags) {
    this.value = value;
    this.tags = tags;
  }

  @Override
  public void unregister() {
    if (0 != gaugePtr) {
      unregisterGauge(gaugePtr);
    }
    gaugePtr = 0;
  }

  public void record() {
    Preconditions.checkState(gaugePtr != 0, "Gauge native pointer must not be 0.");
    // Get tag key list from map;
    List<TagKey> nativeTagKeyList = new ArrayList<>();
    List<String> tagValues = new ArrayList<>();
    for (Map.Entry<TagKey, String> entry : tags.entrySet()) {
      nativeTagKeyList.add(entry.getKey());
      tagValues.add(entry.getValue());
    }
    // Get tag value list from map;
    recordNative(gaugePtr, value, nativeTagKeyList.stream()
        .map(TagKey::getTagKey).collect(Collectors.toList()), tagValues);
  }

  private native void recordNative(long gaugePtr, double value,
                             List tagKeys,
                             List<String> tagValues);

  @Override
  public String toString() {
    return "Gauge{" +
      "name='" + name + '\'' +
      ", value=" + value +
      ", gaugePtr=" + gaugePtr +
      ", tags=" + tags +
      '}';
  }

}

