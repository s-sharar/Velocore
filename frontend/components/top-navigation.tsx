"use client"

import { useState, useEffect } from "react"
import Link from "next/link"
import { usePathname } from "next/navigation"
import { Button } from "@/components/ui/button"
import { Badge } from "@/components/ui/badge"
import { cn } from "@/lib/utils"
import { TrendingUp, Home, BookOpen, BarChart3, Activity, Settings, Wifi, WifiOff, Bell, User } from "lucide-react"
import { motion, AnimatePresence } from "framer-motion"

const navigation = [
  { name: "Dashboard", href: "/", icon: Home },
  { name: "Trading", href: "/orders", icon: BookOpen },
  { name: "Order Book", href: "/orderbook", icon: BarChart3 },
  { name: "Market Data", href: "/market", icon: TrendingUp },
  { name: "Analytics", href: "/statistics", icon: Activity },
]

interface ConnectionStatus {
  connected: boolean
  subscribed_symbols: string[]
}

export function TopNavigation() {
  const pathname = usePathname()
  const [status, setStatus] = useState<ConnectionStatus>({
    connected: false,
    subscribed_symbols: [],
  })

  useEffect(() => {
    const checkStatus = async () => {
      try {
        const response = await fetch("http://localhost:18080/market/status")
        if (response.ok) {
          const data = await response.json()
          setStatus(data)
        }
      } catch (error) {
        setStatus({ connected: false, subscribed_symbols: [] })
      }
    }

    checkStatus()
    const interval = setInterval(checkStatus, 5000)
    return () => clearInterval(interval)
  }, [])

  return (
    <motion.nav
      initial={{ y: -100 }}
      animate={{ y: 0 }}
      className="fixed top-0 left-0 right-0 z-50 bg-[#131722]/95 backdrop-blur-xl border-b border-[#2A2E39]"
    >
      <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
        <div className="flex items-center justify-between h-16">
          {/* Logo */}
          <motion.div
            className="flex items-center gap-3"
            whileHover={{ scale: 1.02 }}
            transition={{ type: "spring", stiffness: 400, damping: 17 }}
          >
            <div className="relative">
              <div className="w-8 h-8 bg-gradient-to-r from-[#2962FF] to-[#00D4AA] rounded-lg flex items-center justify-center">
                <TrendingUp className="h-5 w-5 text-white" />
              </div>
            </div>
            <div>
              <h1 className="text-xl font-semibold text-white">Velocore</h1>
              <p className="text-xs text-[#787B86] -mt-1">Trading Simulator</p>
            </div>
          </motion.div>

          {/* Navigation Links */}
          <div className="hidden md:flex items-center space-x-1">
            {navigation.map((item) => {
              const isActive = pathname === item.href
              return (
                <Link key={item.name} href={item.href}>
                  <motion.div
                    className={cn(
                      "relative px-4 py-2 rounded-lg text-sm font-medium transition-all duration-200",
                      isActive ? "text-white bg-[#1E222D]" : "text-[#787B86] hover:text-white hover:bg-[#1E222D]/50",
                    )}
                    whileHover={{ scale: 1.02 }}
                    whileTap={{ scale: 0.98 }}
                  >
                    <div className="flex items-center gap-2">
                      <item.icon className="h-4 w-4" />
                      {item.name}
                    </div>
                    {isActive && (
                      <motion.div
                        layoutId="activeTab"
                        className="absolute bottom-0 left-0 right-0 h-0.5 bg-[#2962FF] rounded-full"
                        transition={{ type: "spring", stiffness: 500, damping: 30 }}
                      />
                    )}
                  </motion.div>
                </Link>
              )
            })}
          </div>

          {/* Right Side */}
          <div className="flex items-center gap-3">
            {/* Connection Status */}
            <motion.div
              animate={{
                scale: status.connected ? [1, 1.05, 1] : 1,
              }}
              transition={{
                duration: status.connected ? 2 : 0,
                repeat: status.connected ? Number.POSITIVE_INFINITY : 0,
              }}
            >
              <Badge
                variant={status.connected ? "default" : "destructive"}
                className={cn(
                  "gap-2 px-3 py-1 text-xs font-medium",
                  status.connected
                    ? "bg-[#00D4AA]/10 text-[#00D4AA] border-[#00D4AA]/20 hover:bg-[#00D4AA]/20"
                    : "bg-[#FF5252]/10 text-[#FF5252] border-[#FF5252]/20",
                )}
              >
                {status.connected ? <Wifi className="h-3 w-3" /> : <WifiOff className="h-3 w-3" />}
                {status.connected ? "Live" : "Offline"}
              </Badge>
            </motion.div>

            {/* Notifications */}
            <Button
              variant="ghost"
              size="sm"
              className="relative text-[#787B86] hover:text-white hover:bg-[#1E222D]/50 h-8 w-8 p-0"
            >
              <Bell className="h-4 w-4" />
              <motion.div
                className="absolute -top-1 -right-1 h-2 w-2 bg-[#FF5252] rounded-full"
                animate={{ scale: [1, 1.2, 1] }}
                transition={{ duration: 2, repeat: Number.POSITIVE_INFINITY }}
              />
            </Button>

            {/* Settings */}
            <Button
              variant="ghost"
              size="sm"
              className="text-[#787B86] hover:text-white hover:bg-[#1E222D]/50 h-8 w-8 p-0"
            >
              <Settings className="h-4 w-4" />
            </Button>

            {/* Profile */}
            <Button
              variant="ghost"
              size="sm"
              className="text-[#787B86] hover:text-white hover:bg-[#1E222D]/50 h-8 w-8 p-0"
            >
              <User className="h-4 w-4" />
            </Button>
          </div>
        </div>
      </div>

      {/* Subscribed Symbols Ticker */}
      <AnimatePresence>
        {status.subscribed_symbols.length > 0 && (
          <motion.div
            initial={{ height: 0, opacity: 0 }}
            animate={{ height: "auto", opacity: 1 }}
            exit={{ height: 0, opacity: 0 }}
            className="border-t border-[#2A2E39] bg-[#1E222D]/50 backdrop-blur-sm overflow-hidden"
          >
            <div className="px-4 py-2">
              <div className="flex items-center gap-2 text-xs text-[#787B86]">
                <span>Subscribed:</span>
                <div className="flex gap-2">
                  {status.subscribed_symbols.map((symbol) => (
                    <Badge key={symbol} variant="outline" className="text-xs border-[#2A2E39] text-[#787B86]">
                      {symbol}
                    </Badge>
                  ))}
                </div>
              </div>
            </div>
          </motion.div>
        )}
      </AnimatePresence>
    </motion.nav>
  )
}
