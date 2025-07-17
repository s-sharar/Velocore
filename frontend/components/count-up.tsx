"use client"

import { useEffect, useState } from "react"
import { motion, useSpring, useTransform } from "framer-motion"

interface CountUpProps {
  end: number
  decimals?: number
  duration?: number
}

export function CountUp({ end, decimals = 0, duration = 1 }: CountUpProps) {
  const [displayValue, setDisplayValue] = useState(0)
  const spring = useSpring(0, { stiffness: 100, damping: 30 })
  const display = useTransform(spring, (value) => value.toFixed(decimals))

  useEffect(() => {
    spring.set(end)
  }, [spring, end])

  useEffect(() => {
    const unsubscribe = display.onChange((latest) => {
      setDisplayValue(Number.parseFloat(latest))
    })
    return unsubscribe
  }, [display])

  return (
    <motion.span initial={{ opacity: 0, y: 20 }} animate={{ opacity: 1, y: 0 }} transition={{ duration: 0.5 }}>
      {displayValue.toFixed(decimals)}
    </motion.span>
  )
}
