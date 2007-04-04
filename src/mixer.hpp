// Copyright 2007 Ben Hutchings and Tore Sinding Bekkedal.
// See the file "COPYING" for licence details.

#ifndef DVSWITCH_MIXER_HPP
#define DVSWITCH_MIXER_HPP

#include <cstddef>
#include <vector>

#include <tr1/memory>

#include <boost/thread/mutex.hpp>

#include "ring_buffer.hpp"

namespace boost
{
    class thread;
}

class frame;

class mixer
{
public:
    // Identifiers to distinguish mixer's sources and sinks
    typedef unsigned source_id, sink_id;
    // Reference-counting pointer to a frame
    typedef std::tr1::shared_ptr<frame> frame_ptr;

    // Interface to sinks
    struct sink
    {
	// Put a frame out.
	// The frame is shared with other sinks and must not be
	// modified.  It should be released as soon as possible.
	// This will be called at the appropriate frame rate even
	// if there are no new frames available.  The serial_num
	// member of the frame can be used to check whether the
	// frame is new.
	virtual void put_frame(const frame_ptr &) = 0;
	// Make a cut in the output, if appropriate
	virtual void cut() = 0;
    };

    // Interface to monitor
    struct monitor
    {
	virtual void put_frames(unsigned source_count,
				const frame_ptr * source_frames,
				const frame_ptr & mixed_frame) = 0;
    };

    mixer();
    ~mixer();

    // Interface for sources
    // Register and unregister sources
    source_id add_source();
    void remove_source(source_id);
    // Allocate a frame buffer.  This uses a memory pool and should be
    // fast.
    static frame_ptr allocate_frame();
    // Add a new frame from the given source.  This should be called at
    // appropriate intervals to avoid the need to drop or duplicate
    // frames.
    void put_frame(source_id, const frame_ptr &);

    // Interface for sinks
    // Register and unregister sinks
    sink_id add_sink(sink *);
    void remove_sink(sink_id);

    // Interface for monitors
    void set_monitor(monitor *);

    // Mixer interface
    // Select the video source for output
    void set_video_source(source_id);
    // Make a cut in the output as soon as possible, where appropriate
    // for the sink
    void cut();

private:
    // XXX this is rather arbitrary
    static const unsigned ring_buffer_size = 10;

    typedef ring_buffer<frame_ptr, ring_buffer_size> frame_queue;

    // Settings for mixing/switching.  Rather simple at present. ;-)
    // If and when we do real mixing, these will need to be preserved
    // in a queue for the mixing thread(s) to apply before handing off
    // to the sinks.
    struct mix_settings
    {
	source_id video_source_id;
	bool cut_before;
    };

    void start_clock(); // start the clock thread
    void stop_clock();  // stop the clock thread
    void run_clock();   // clock thread function

    boost::mutex source_mutex_; // controls access to the following
    mix_settings settings_;
    std::vector<frame_queue> source_queues_;

    boost::mutex sink_mutex_; // controls access to the following
    std::vector<sink *> sinks_;

    monitor * monitor_;

    boost::thread * clock_thread_;
};

#endif // !defined(DVSWITCH_MIXER_HPP)