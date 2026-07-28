"use client"

import { useState, useEffect } from "react"
import Link from "next/link"
import { usePathname } from "next/navigation"
import { Badge } from "@/components/ui/badge"
import { cn } from "@/lib/utils"
import { Home, BookOpen, Wifi, WifiOff, TrendingUp, Bookmark, User } from "lucide-react"
import { motion, AnimatePresence } from "framer-motion"

const navigation = [
  { name: "Dashboard", href: "/", icon: Home },
  { name: "Portfolio", href: "/orders", icon: BookOpen },
  { name: "Payout", href: "/orderbook", icon: TrendingUp },
  { name: "Bookmarks", href: "/market", icon: Bookmark },
  { name: "Profile", href: "/statistics", icon: User },
]

interface SidebarProps {
  isOpen: boolean
  onToggle: () => void
}

interface ConnectionStatus {
  connected: boolean
  subscribed_symbols: string[]
}

export function Sidebar({ isOpen }: SidebarProps) {
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
    <motion.div
      initial={false}
      animate={{ width: isOpen ? 256 : 64 }}
      transition={{ duration: 0.3, ease: "easeInOut" }}
      className="fixed inset-y-0 left-0 z-50 bg-gray-900 border-r border-gray-800"
    >
      <div className="flex flex-col h-full">
        {/* Logo area */}
        <div className="flex items-center justify-center h-16 border-b border-gray-800">
          <AnimatePresence mode="wait">
            {isOpen ? (
              <motion.div
                key="logo-full"
                initial={{ opacity: 0 }}
                animate={{ opacity: 1 }}
                exit={{ opacity: 0 }}
                transition={{ duration: 0.2 }}
                className="flex items-center gap-3"
              >
                <div className="w-8 h-8 bg-gradient-to-r from-blue-600 to-purple-600 rounded-lg flex items-center justify-center">
                  <span className="text-white font-bold text-sm">V</span>
                </div>
                <span className="text-white font-semibold">Velocore</span>
              </motion.div>
            ) : (
              <motion.div
                key="logo-mini"
                initial={{ opacity: 0 }}
                animate={{ opacity: 1 }}
                exit={{ opacity: 0 }}
                transition={{ duration: 0.2 }}
                className="w-8 h-8 bg-gradient-to-r from-blue-600 to-purple-600 rounded-lg flex items-center justify-center"
              >
                <span className="text-white font-bold text-sm">V</span>
              </motion.div>
            )}
          </AnimatePresence>
        </div>

        {/* Navigation */}
        <nav className="flex-1 px-3 py-6 space-y-2">
          {navigation.map((item) => {
            const isActive = pathname === item.href
            return (
              <Link key={item.name} href={item.href}>
                <motion.div
                  className={cn(
                    "flex items-center gap-3 px-3 py-3 rounded-lg text-sm font-medium transition-all duration-200 group",
                    isActive
                      ? "bg-gray-800 text-white border border-gray-700"
                      : "text-gray-400 hover:text-white hover:bg-gray-800",
                  )}
                  whileHover={{ scale: 1.02 }}
                  whileTap={{ scale: 0.98 }}
                >
                  <item.icon
                    className={cn(
                      "h-5 w-5 flex-shrink-0",
                      isActive ? "text-blue-400" : "text-gray-500 group-hover:text-gray-300",
                    )}
                  />
                  <AnimatePresence>
                    {isOpen && (
                      <motion.span
                        initial={{ opacity: 0, width: 0 }}
                        animate={{ opacity: 1, width: "auto" }}
                        exit={{ opacity: 0, width: 0 }}
                        transition={{ duration: 0.2 }}
                        className="truncate"
                      >
                        {item.name}
                      </motion.span>
                    )}
                  </AnimatePresence>
                </motion.div>
              </Link>
            )
          })}
        </nav>

        {/* Connection Status */}
        <AnimatePresence>
          {isOpen && (
            <motion.div
              initial={{ opacity: 0, height: 0 }}
              animate={{ opacity: 1, height: "auto" }}
              exit={{ opacity: 0, height: 0 }}
              transition={{ duration: 0.2 }}
              className="px-3 py-4 border-t border-gray-800"
            >
              <div className="flex items-center justify-between mb-3">
                <span className="text-sm font-medium text-gray-300">Connection</span>
                <Badge
                  className={cn(
                    "text-xs",
                    status.connected
                      ? "bg-green-900 text-green-300 border-green-700"
                      : "bg-red-900 text-red-300 border-red-700",
                  )}
                >
                  {status.connected ? <Wifi className="h-3 w-3 mr-1" /> : <WifiOff className="h-3 w-3 mr-1" />}
                  {status.connected ? "Live" : "Offline"}
                </Badge>
              </div>

              {status.subscribed_symbols.length > 0 && (
                <div className="space-y-2">
                  <span className="text-xs text-gray-500">Subscribed</span>
                  <div className="flex flex-wrap gap-1">
                    {status.subscribed_symbols.slice(0, 2).map((symbol) => (
                      <Badge key={symbol} variant="outline" className="text-xs border-gray-700 text-gray-400">
                        {symbol}
                      </Badge>
                    ))}
                    {status.subscribed_symbols.length > 2 && (
                      <Badge variant="outline" className="text-xs border-gray-700 text-gray-400">
                        +{status.subscribed_symbols.length - 2}
                      </Badge>
                    )}
                  </div>
                </div>
              )}
            </motion.div>
          )}
        </AnimatePresence>
      </div>
    </motion.div>
  )
}
