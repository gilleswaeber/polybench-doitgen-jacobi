
# This script reads the lsb data and export csv summaries

library(ggplot2)
library(dplyr)

setwd("/home/quentin/Desktop/dphpc-project/R")

source("utils.R")
source("stats.R")

data.bh_study <-  ReadAllFilesInDir.Aggregate(dir.path="data2/", col=c("NR", "NQ", "NP", "num_processes", "run_index", "benchmark_type", "processor_model", "id", "time", "overhead"))

init_data <- subset(data.bh_study, id == 0 & benchmark_type != "sequential")
kernel_data <- subset(data.bh_study, id == 1)
write_data <- subset(data.bh_study, id == 2)

init_summary <- CalculateDataSummary(data=init_data, measurevar="time", groupvars=c("NR", "NQ", "NP", "num_processes", "id", "benchmark_type"), conf.interval=.95, quantile.interval=.95)
kernel_summary <- CalculateDataSummary(data=kernel_data, measurevar="time", groupvars=c("NR", "NQ", "NP", "num_processes", "id"), conf.interval=.95, quantile.interval=.95)


data_group <- data.bh_study %>% group_by(NR, NQ, NP, num_processes, run_index, benchmark_type)
result_data <-data_group %>% dplyr::summarise(total = sum(time, na.rm = TRUE))
total_summary <- CalculateDataSummary(data=result_data, measurevar="total", groupvars=c("NR", "NQ", "NP", "num_processes", "benchmark_type"), conf.interval=.95, quantile.interval=.95)

write.table(init_summary, file = "init_summary.txt", sep = "\t", quote = FALSE, row.names = F)
write.table(kernel_summary, file = "kernel_summary.txt", sep = "\t", quote = FALSE, row.names = F)
write.table(total_summary, file = "overall_summary.txt", sep = "\t", quote = FALSE, row.names = F)
write.table(write_data, file = "io_summary.txt", sep = "\t", quote = FALSE, row.names = F)
