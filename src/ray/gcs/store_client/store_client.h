#ifndef RAY_GCS_STORE_CLIENT_H
#define RAY_GCS_STORE_CLIENT_H

#include <memory>
#include <string>
#include "ray/gcs/callback.h"
#include "ray/util/io_service_pool.h"

namespace ray {

namespace gcs {

/// \class StoreClient
/// Abstract interface of the storage client.
///
/// To read and write from the storage, `Connect()` must be called and return Status::OK.
/// Before exit, `Disconnect()` must be called.
class StoreClient {
 public:
  virtual ~StoreClient() {}

  /// Connect to storage. Non-thread safe.
  ///
  /// \return Status
  virtual Status Connect(std::shared_ptr<IOServicePool> io_service_pool) = 0;

  /// Disconnect with storage. Non-thread safe.
  virtual void Disconnect() = 0;

  /// Write data to table asynchronously.
  ///
  /// \param table_name The name of the table to be write.
  /// \param key The key that will be write to the table.
  /// \param value The value of the key that will be write to the table.
  /// \param callback Callback that will be called after write finishes.
  /// \return Status
  virtual Status AsyncPut(const std::string &table_name, const std::string &key,
                          const std::string &value, const StatusCallback &callback) = 0;

  /// Write data to table asynchronously.
  ///
  /// \param table_name The name of the table to be write.
  /// \param key The key that will be write to the table.
  /// \param index The index value map to the key that will be write to the table.
  /// \param value The value of the key that will be write to the table.
  /// \param callback Callback that will be called after write finishes.
  /// \return Status
  virtual Status AsyncPut(const std::string &table_name, const std::string &key,
                          const std::string &index, const std::string &value,
                          const StatusCallback &callback) = 0;

  /// Get data from table asynchronously.
  ///
  /// \param table_name The name of the table to be read.
  /// \param key The key that will be read from the table.
  /// \param callback Callback that will be called after read finishes.
  /// \return Status
  virtual Status AsyncGet(const std::string &table_name, const std::string &key,
                          const OptionalItemCallback<std::string> &callback) = 0;

  /// Get data by index from table asynchronously.
  ///
  /// \param table_name The name of the table to be read.
  /// \param key The key that will be read from the table.
  /// \param callback Callback that will be called after read finishes.
  /// \return Status
  virtual Status AsyncGetByIndex(const std::string &table_name, const std::string &index,
                                 const MultiItemCallback &callback) = 0;

  /// Get all data from table asynchronously.
  ///
  /// \param table_name The name of the table to be read.
  /// \param callback Callback that will be called when receives data of the table.
  /// If the callback return `has_more == true` mean there's more data will be received.
  /// \return Status
  virtual Status AsyncGetAll(const std::string &table_name,
                             const ScanCallback<std::string> &callback) = 0;

  /// Delete data from table asynchronously.
  ///
  /// \param table_name The name of the table which data is to be deleted.
  /// \param key The key that will be deleted from the table.
  /// \param callback Callback that will be called after delete finishes.
  /// \return Status
  virtual Status AsyncDelete(const std::string &table_name, const std::string &key,
                             const StatusCallback &callback) = 0;

  /// Delete by index from table asynchronously.
  ///
  /// \param table_name The name of the table which data is to be deleted.
  /// \param index The index of the key that will be deleted from the table.
  /// \param callback Callback that will be called after delete finishes.
  /// \return Status
  virtual Status AsyncDeleteByIndex(const std::string &table_name,
                                    const std::string &index,
                                    const StatusCallback &callback) = 0;

 protected:
  StoreClient() {}

  std::shared_ptr<IOServicePool> io_service_pool_;
};

}  // namespace gcs

}  // namespace ray

#endif  // RAY_GCS_STORE_CLIENT_H
