package io.ray.runtime.metric;

import java.util.Objects;

/**
 * Tagkey mapping java object to stats tagkey object.
 */
public class TagKey {

  private String tagKey;

  public TagKey(String key) {
    tagKey =  key;
    registerTagkeyNative(key);
  }

  public String getTagKey() {
    return tagKey;
  }

  @Override
  public boolean equals(Object o) {
    if (this == o) {
      return true;
    }
    if (!(o instanceof TagKey)) {
      return false;
    }
    TagKey tagKey1 = (TagKey) o;
    return Objects.equals(tagKey, tagKey1.tagKey);
  }

  @Override
  public int hashCode() {
    return Objects.hash(tagKey);
  }

  private native void registerTagkeyNative(String tagKey);

  @Override
  public String toString() {
    return "TagKey{" +
      ", tagKey='" + tagKey + '\'' +
      '}';
  }
}