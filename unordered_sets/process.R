## Process unordered_set benchmark results in results.tsv.
## Generates many PDF files containing boxplots of benchmark scenarios.

library(ggplot2)
library(dplyr)
library(rlang)

# read input file
data = read.csv(file='results.tsv', sep="\t")

# calculate new columns per item
data$time_per_item <- data$time / data$size * 1e6
data$cycles_per_item <- data$cpu_cycles / data$size
data$instructions_per_item <- data$instructions / data$size
data$ref_cycles_per_item <- data$ref_cpu_cycles / data$size
data$l1i_read_miss_per_item <- data$l1i_read_miss / data$size
data$l1d_read_miss_per_item <- data$l1d_read_miss / data$size
data$ll_read_miss_per_item <- data$ll_read_miss / data$size

# factor by size
data$size <- as.factor(data$size)

head(data)

# aesthetic to join medians of boxplots
my_aes <- stat_summary(
    geom='line', fun.y=median, position=position_dodge(0.75),
    aes(group=factor(container), color=container))

# theme with larger black fonts
theme_update(
    text = element_text(size=12),
    plot.title = element_text(size=14, hjust=0.5),
    axis.text.x = element_text(size=10, colour="black",
                               angle=90, hjust=1, vjust=0.5),
    axis.text.y = element_text(size=10, colour="black"),
    strip.text.x = element_text(size=10, margin=margin(0.7,0,0.7,0,"mm")),
    legend.title = element_text(size=10),
    legend.text = element_text(size=10))

# filtering function - turns outliers into NAs to be removed
filter_outliers <- function(x) {
    l <- boxplot.stats(x)$stats[1]
    u <- boxplot.stats(x)$stats[5]

    for (i in 1:length(x)) {
        x[i] <- ifelse(x[i] > l & x[i] < u, x[i], NA)
    }
    return (x)
}

# plot single datasets
plot_dataset <- function(filter, filter_title, col, colname) {
    # select benchmark scenario for this plot
    df <- data[data$benchmark==filter, ]
    # copy column "col" into value
    df$value <- df[ ,col]

    # remove all outliers from value column, store as valuex
    df_no_outliers <- df %>%
        group_by(size, container) %>%
        mutate(valuex = filter_outliers(value))

    # create virtual plot without outliers using valuex
    p0 <- ggplot(df_no_outliers, aes(x=size, y=valuex, color=container)) +
        geom_boxplot() + my_aes +
        ggtitle(paste(filter_title, "-", colname)) +
        scale_x_discrete("Input Size") +
        scale_y_continuous(colname)

    # get min/max range of virtual plot without outliers
    ylims <- ggplot_build(p0)$layout$panel_scales_y[[1]]$range$range

    # real plot with calculated limits without outliers
    p1 <- ggplot(df, aes(x=size, y=value, color=container)) +
        geom_boxplot() + my_aes +
        ggtitle(paste(filter_title, "-", colname)) +
        scale_x_discrete("Input Size") +
        scale_y_continuous(colname) +
        coord_cartesian(ylim = ylims * 1.05)

    # save plot
    ggsave(p1, filename=paste0(filter, " - ", col, "_plot.pdf"),
           width=10, height=8)
}

# multiplot various benchmark scenarios: insert, find, and insert-find-delete
plot_benchmark <- function(container, cname, benchmark, bname) {
    plot_dataset(paste0(container, '_insert'),
                 paste(cname, 'Insert'),
                 benchmark, bname)

    plot_dataset(paste(container, '_find', sep=''),
                 paste(cname, 'Find'),
                 benchmark, bname)

    plot_dataset(paste(container, '_insert_find_delete', sep=''),
                 paste(cname, 'Insert, Find, Delete'),
                 benchmark, bname)
}

# multiplot various metrics
plot_set_map <- function(container, cname) {
    plot_benchmark(container, cname,
                   'time_per_item', 'Wallclock Time in Microseconds per Item')

    plot_benchmark(container, cname,
                   'cycles_per_item', 'CPU Cycles per Item')

    plot_benchmark(container, cname,
                   'instructions_per_item', 'CPU Instructions per Item')

    plot_benchmark(container, cname,
                   'ref_cycles_per_item', 'Ref CPU Cycles per Item')

    plot_benchmark(container, cname,
                   'l1i_read_miss_per_item', 'L1 Instruction Cache Misses per Item')

    plot_benchmark(container, cname,
                   'l1d_read_miss_per_item', 'L1 Data Cache Misses per Item')

    plot_benchmark(container, cname,
                   'll_read_miss_per_item', 'LL Data Cache Misses per Item')
}

# do the actual plotting
plot_set_map('set', 'Set')

plot_set_map('map', 'Map')
