

/**** Dummy IIO data structs ***/

static const struct config_item_type iio_dummy_type = {
    .ct_owner = THIS_MODULE,
};

/**
 * struct iio_dummy_accel_calibscale - realworld to register mapping
 * @val: first value in read_raw - here integer part.
 * @val2: second value in read_raw etc - here micro part.
 * @regval: register value - magic device specific numbers.
 */
struct iio_dummy_accel_calibscale {
    int val;
    int val2;
    int regval; /* what would be written to hardware */
};

static const struct iio_dummy_accel_calibscale dummy_scales[] = {
    { 0, 100, 0x8 }, /* 0.000100 */
    { 0, 133, 0x7 }, /* 0.000133 */
    { 733, 13, 0x9 }, /* 733.000013 */
};

#ifdef CONFIG_IIO_SIMPLE_DUMMY_EVENTS

/*
 * simple event - triggered when value rises above
 * a threshold
 */
static const struct iio_event_spec iio_dummy_event = {
    .type = IIO_EV_TYPE_THRESH,
    .dir = IIO_EV_DIR_RISING,
    .mask_separate = BIT(IIO_EV_INFO_VALUE) | BIT(IIO_EV_INFO_ENABLE),
};


/*
 * simple step detect event - triggered when a step is detected
 */
static const struct iio_event_spec step_detect_event = {
    .type = IIO_EV_TYPE_CHANGE,
    .dir = IIO_EV_DIR_NONE,
    .mask_separate = BIT(IIO_EV_INFO_ENABLE),
};

/*
 * simple transition event - triggered when the reported running confidence
 * value rises above a threshold value
 */
static const struct iio_event_spec iio_running_event = {
    .type = IIO_EV_TYPE_THRESH,
    .dir = IIO_EV_DIR_RISING,
    .mask_separate = BIT(IIO_EV_INFO_VALUE) | BIT(IIO_EV_INFO_ENABLE),
};

/*
 * simple transition event - triggered when the reported walking confidence
 * value falls under a threshold value
 */
static const struct iio_event_spec iio_walking_event = {
    .type = IIO_EV_TYPE_THRESH,
    .dir = IIO_EV_DIR_FALLING,
    .mask_separate = BIT(IIO_EV_INFO_VALUE) | BIT(IIO_EV_INFO_ENABLE),
};
#endif


/*
 * iio_dummy_channels - Description of available channels
 *
 * This array of structures tells the IIO core about what the device
 * actually provides for a given channel.
 */
static const struct iio_chan_spec iio_dummy_channels[] = {
    /* indexed ADC channel in_voltage0_raw etc */
    {
        .type = IIO_VOLTAGE,
        /* Channel has a numeric index of 0 */
        .indexed = 1,
        .channel = 0,
        /* What other information is available? */
        .info_mask_separate =
        /*
         * in_voltage0_raw
         * Raw (unscaled no bias removal etc) measurement
         * from the device.
         */
        BIT(IIO_CHAN_INFO_RAW) |
        /*
         * in_voltage0_offset
         * Offset for userspace to apply prior to scale
         * when converting to standard units (microvolts)
         */
        BIT(IIO_CHAN_INFO_OFFSET) |
        /*
         * in_voltage0_scale
         * Multipler for userspace to apply post offset
         * when converting to standard units (microvolts)
         */
        BIT(IIO_CHAN_INFO_SCALE),
        /*
         * sampling_frequency
         * The frequency in Hz at which the channels are sampled
         */
        .info_mask_shared_by_dir = BIT(IIO_CHAN_INFO_SAMP_FREQ),
        /* The ordering of elements in the buffer via an enum */
        .scan_index = DUMMY_INDEX_VOLTAGE_0,
        .scan_type = { /* Description of storage in buffer */
            .sign = 'u', /* unsigned */
            .realbits = 13, /* 13 bits */
            .storagebits = 16, /* 16 bits used for storage */
            .shift = 0, /* zero shift */
        },
#ifdef CONFIG_IIO_SIMPLE_DUMMY_EVENTS
        .event_spec = &iio_dummy_event,
        .num_event_specs = 1,
#endif /* CONFIG_IIO_SIMPLE_DUMMY_EVENTS */
    },
    /* Differential ADC channel in_voltage1-voltage2_raw etc*/
    {
        .type = IIO_VOLTAGE,
        .differential = 1,
        /*
         * Indexing for differential channels uses channel
         * for the positive part, channel2 for the negative.
         */
        .indexed = 1,
        .channel = 1,
        .channel2 = 2,
        /*
         * in_voltage1-voltage2_raw
         * Raw (unscaled no bias removal etc) measurement
         * from the device.
         */
        .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
        /*
         * in_voltage-voltage_scale
         * Shared version of scale - shared by differential
         * input channels of type IIO_VOLTAGE.
         */
        .info_mask_shared_by_type = BIT(IIO_CHAN_INFO_SCALE),
        /*
         * sampling_frequency
         * The frequency in Hz at which the channels are sampled
         */
        .scan_index = DUMMY_INDEX_DIFFVOLTAGE_1M2,
        .scan_type = { /* Description of storage in buffer */
            .sign = 's', /* signed */
            .realbits = 12, /* 12 bits */
            .storagebits = 16, /* 16 bits used for storage */
            .shift = 0, /* zero shift */
        },
    },
    /* Differential ADC channel in_voltage3-voltage4_raw etc*/
    {
        .type = IIO_VOLTAGE,
        .differential = 1,
        .indexed = 1,
        .channel = 3,
        .channel2 = 4,
        .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
        .info_mask_shared_by_type = BIT(IIO_CHAN_INFO_SCALE),
        .info_mask_shared_by_dir = BIT(IIO_CHAN_INFO_SAMP_FREQ),
        .scan_index = DUMMY_INDEX_DIFFVOLTAGE_3M4,
        .scan_type = {
            .sign = 's',
            .realbits = 11,
            .storagebits = 16,
            .shift = 0,
        },
    },
    /*
     * 'modified' (i.e. axis specified) acceleration channel
     * in_accel_z_raw
     */
    {
        .type = IIO_ACCEL,
        .modified = 1,
        /* Channel 2 is use for modifiers */
        .channel2 = IIO_MOD_X,
        .info_mask_separate = BIT(IIO_CHAN_INFO_RAW) |
        /*
         * Internal bias and gain correction values. Applied
         * by the hardware or driver prior to userspace
         * seeing the readings. Typically part of hardware
         * calibration.
         */
        BIT(IIO_CHAN_INFO_CALIBSCALE) |
        BIT(IIO_CHAN_INFO_CALIBBIAS),
        .info_mask_shared_by_dir = BIT(IIO_CHAN_INFO_SAMP_FREQ),
        .scan_index = DUMMY_INDEX_ACCELX,
        .scan_type = { /* Description of storage in buffer */
            .sign = 's', /* signed */
            .realbits = 16, /* 16 bits */
            .storagebits = 16, /* 16 bits used for storage */
            .shift = 0, /* zero shift */
        },
    },
    /*
     * Convenience macro for timestamps. 4 is the index in
     * the buffer.
     */
    IIO_CHAN_SOFT_TIMESTAMP(4),
    /* DAC channel out_voltage0_raw */
    {
        .type = IIO_VOLTAGE,
        .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
        .scan_index = -1, /* No buffer support */
        .output = 1,
        .indexed = 1,
        .channel = 0,
    },
    {
        .type = IIO_STEPS,
        .info_mask_shared_by_type = BIT(IIO_CHAN_INFO_ENABLE) |
            BIT(IIO_CHAN_INFO_CALIBHEIGHT),
        .info_mask_separate = BIT(IIO_CHAN_INFO_PROCESSED),
        .scan_index = -1, /* No buffer support */
#ifdef CONFIG_IIO_SIMPLE_DUMMY_EVENTS
        .event_spec = &step_detect_event,
        .num_event_specs = 1,
#endif /* CONFIG_IIO_SIMPLE_DUMMY_EVENTS */
    },
    {
        .type = IIO_ACTIVITY,
        .modified = 1,
        .channel2 = IIO_MOD_RUNNING,
        .info_mask_separate = BIT(IIO_CHAN_INFO_PROCESSED),
        .scan_index = -1, /* No buffer support */
#ifdef CONFIG_IIO_SIMPLE_DUMMY_EVENTS
        .event_spec = &iio_running_event,
        .num_event_specs = 1,
#endif /* CONFIG_IIO_SIMPLE_DUMMY_EVENTS */
    },
    {
        .type = IIO_ACTIVITY,
        .modified = 1,
        .channel2 = IIO_MOD_WALKING,
        .info_mask_separate = BIT(IIO_CHAN_INFO_PROCESSED),
        .scan_index = -1, /* No buffer support */
#ifdef CONFIG_IIO_SIMPLE_DUMMY_EVENTS
        .event_spec = &iio_walking_event,
        .num_event_specs = 1,
#endif /* CONFIG_IIO_SIMPLE_DUMMY_EVENTS */
    },
};

static int iio_dummy_read_raw(struct iio_dev *indio_dev,
                  struct iio_chan_spec const *chan,
                  int *val,
                  int *val2,
                  long mask);

static int iio_dummy_write_raw(struct iio_dev *indio_dev,
                   struct iio_chan_spec const *chan,
                   int val,
                   int val2,
                   long mask);

struct iio_dev;

int zn_iio_simple_dummy_read_event_config(struct iio_dev *indio_dev,
                       const struct iio_chan_spec *chan,
                       enum iio_event_type type,
                       enum iio_event_direction dir);

int zn_iio_simple_dummy_write_event_config(struct iio_dev *indio_dev,
                    const struct iio_chan_spec *chan,
                    enum iio_event_type type,
                    enum iio_event_direction dir,
                    int state);

int zn_iio_simple_dummy_read_event_value(struct iio_dev *indio_dev,
                      const struct iio_chan_spec *chan,
                      enum iio_event_type type,
                      enum iio_event_direction dir,
                      enum iio_event_info info, int *val,
                      int *val2);

int zn_iio_simple_dummy_write_event_value(struct iio_dev *indio_dev,
                       const struct iio_chan_spec *chan,
                       enum iio_event_type type,
                       enum iio_event_direction dir,
                       enum iio_event_info info, int val,
                       int val2);

int zn_iio_simple_dummy_events_register(struct iio_dev *indio_dev);
void zn_iio_simple_dummy_events_unregister(struct iio_dev *indio_dev);



/*
 * Device type specific information.
 */
static const struct iio_info iio_dummy_info = {
    .read_raw = &iio_dummy_read_raw,
    .write_raw = &iio_dummy_write_raw,
#ifdef CONFIG_IIO_SIMPLE_DUMMY_EVENTS
    .read_event_config = &zn_iio_simple_dummy_read_event_config,
    .write_event_config = &zn_iio_simple_dummy_write_event_config,
    .read_event_value = &zn_iio_simple_dummy_read_event_value,
    .write_event_value = &zn_iio_simple_dummy_write_event_value,
#endif /* CONFIG_IIO_SIMPLE_DUMMY_EVENTS */
};


