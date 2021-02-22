#pragma once
#include "fifo_queue.h"
#include "types.h"
#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <vector>

// Uses signed 16-bits samples.

class AudioStream
{
public:
  using SampleType = s16;

  enum : u32
  {
    DefaultInputSampleRate = 44100,
    DefaultOutputSampleRate = 44100,
    DefaultBufferSize = 2048,
    MaxSamples = 32768,
    FullVolume = 100
  };

  AudioStream();
  virtual ~AudioStream();

  u32 GetOutputSampleRate() const { return m_output_sample_rate; }
  u32 GetChannels() const { return m_channels; }
  u32 GetBufferSize() const { return m_buffer_size; }
  s32 GetOutputVolume() const { return m_output_volume; }
  bool IsSyncing() const { return m_sync; }

  bool Reconfigure(u32 input_sample_rate = DefaultInputSampleRate, u32 output_sample_rate = DefaultOutputSampleRate,
                   u32 channels = 1, u32 buffer_size = DefaultBufferSize);
  void SetSync(bool enable) { m_sync = enable; }

  void SetInputSampleRate(u32 sample_rate);

  virtual void SetOutputVolume(u32 volume);

  void PauseOutput(bool paused);
  void EmptyBuffers();

  void Shutdown();

  void BeginWrite(SampleType** buffer_ptr, u32* num_frames);
  void WriteFrames(const SampleType* frames, u32 num_frames);
  void EndWrite(u32 num_frames);

  bool DidUnderflow()
  {
    bool expected = true;
    return m_underflow_flag.compare_exchange_strong(expected, false);
  }

  static std::unique_ptr<AudioStream> CreateNullAudioStream();

  // Latency computation - returns values in seconds
  static float GetMaxLatency(u32 sample_rate, u32 buffer_size);

protected:
  virtual bool OpenDevice() = 0;
  virtual void PauseDevice(bool paused) = 0;
  virtual void CloseDevice() = 0;
  virtual void FramesAvailable() = 0;

  ALWAYS_INLINE static SampleType ApplyVolume(SampleType sample, u32 volume)
  {
    return s16((s32(sample) * s32(volume)) / 100);
  }

  bool SetBufferSize(u32 buffer_size);
  bool IsDeviceOpen() const { return (m_output_sample_rate > 0); }

  u32 GetSamplesAvailable() const;
  u32 GetSamplesAvailableLocked() const;
  void ReadFrames(SampleType* samples, u32 num_frames, bool apply_volume);
  void DropFrames(u32 count);

  u32 m_input_sample_rate = 0;
  u32 m_output_sample_rate = 0;
  u32 m_channels = 0;
  u32 m_buffer_size = 0;

  // volume, 0-100
  u32 m_output_volume = FullVolume;

private:
  ALWAYS_INLINE u32 GetBufferSpace() const { return (m_max_samples - m_buffer.GetSize()); }
  void EnsureBuffer(u32 size);

  void CreateResampler();
  void DestroyResampler();
  void ResetResampler();
  void ResampleInput();

  HeapFIFOQueue<SampleType, MaxSamples> m_buffer;
  mutable std::mutex m_buffer_mutex;
  std::condition_variable m_buffer_draining_cv;
  std::vector<SampleType> m_resample_buffer;

  std::atomic_bool m_underflow_flag{false};
  u32 m_max_samples = 0;

  bool m_output_paused = true;
  bool m_sync = true;

  // Resampling
  double m_resampler_ratio = 1.0;
  void* m_resampler_state = nullptr;
  HeapFIFOQueue<SampleType, MaxSamples> m_resampled_buffer;
  std::vector<float> m_resample_in_buffer;
  std::vector<float> m_resample_out_buffer;
};