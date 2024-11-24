#pragma once

#define m_PIT_FREQUENCY 1193181 //PIT clock frequency in Hz
#define m_PIT_IRQ_PER_SECOND 1000   //How many PIT interrupts per second
#define m_PIT_RESET_TIME (m_PIT_FREQUENCY/m_PIT_IRQ_PER_SECOND)
