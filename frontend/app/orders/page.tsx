"use client"

import type React from "react"
import { useState, useEffect } from "react"
import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card"
import { Button } from "@/components/ui/button"
import { Input } from "@/components/ui/input"
import { Label } from "@/components/ui/label"
import { Select, SelectContent, SelectItem, SelectTrigger, SelectValue } from "@/components/ui/select"
import { Badge } from "@/components/ui/badge"
import { useToast } from "@/hooks/use-toast"
import { BookOpen, Plus, X, TrendingUp, TrendingDown, Loader2 } from "lucide-react"
import { Table, TableBody, TableCell, TableHead, TableHeader, TableRow } from "@/components/ui/table"
import { motion, AnimatePresence } from "framer-motion"

interface Order {
  id: number
  client_id: number
  symbol: string
  side: string
  type: string
  price: number
  quantity: number
  remaining_quantity: number
  filled_quantity: number
  fill_percentage: number
  status: string
  timestamp: number
}

interface OrderFormData {
  symbol: string
  side: string
  type: string
  price: string
  quantity: string
}

export default function OrdersPage() {
  const [formData, setFormData] = useState<OrderFormData>({
    symbol: "AAPL",
    side: "BUY",
    type: "LIMIT",
    price: "",
    quantity: "",
  })
  const [orders, setOrders] = useState<Order[]>([])
  const [loading, setLoading] = useState(false)
  const [submitting, setSubmitting] = useState(false)
  const { toast } = useToast()

  const fetchOrders = async () => {
    try {
      setLoading(true)
      const response = await fetch("http://localhost:18080/orderbook")
      if (response.ok) {
        const data = await response.json()
        const allOrders: Order[] = []

        data.orderbook.bids?.forEach((level: any, index: number) => {
          allOrders.push({
            id: index + 1000,
            client_id: 1,
            symbol: "SIM",
            side: "BUY",
            type: "LIMIT",
            price: level.price,
            quantity: level.quantity,
            remaining_quantity: level.quantity,
            filled_quantity: 0,
            fill_percentage: 0,
            status: "ACTIVE",
            timestamp: Date.now(),
          })
        })

        data.orderbook.asks?.forEach((level: any, index: number) => {
          allOrders.push({
            id: index + 2000,
            client_id: 1,
            symbol: "SIM",
            side: "SELL",
            type: "LIMIT",
            price: level.price,
            quantity: level.quantity,
            remaining_quantity: level.quantity,
            filled_quantity: 0,
            fill_percentage: 0,
            status: "ACTIVE",
            timestamp: Date.now(),
          })
        })

        setOrders(allOrders)
      }
    } catch (error) {
      console.error("Failed to fetch orders:", error)
    } finally {
      setLoading(false)
    }
  }

  useEffect(() => {
    fetchOrders()
    const interval = setInterval(fetchOrders, 5000)
    return () => clearInterval(interval)
  }, [])

  const handleSubmit = async (e: React.FormEvent) => {
    e.preventDefault()
    setSubmitting(true)

    try {
      const orderData = {
        client_id: Math.floor(Math.random() * 1000),
        symbol: formData.symbol,
        side: formData.side,
        type: formData.type,
        price: Number.parseFloat(formData.price),
        quantity: Number.parseInt(formData.quantity),
      }

      const response = await fetch("http://localhost:18080/orders", {
        method: "POST",
        headers: {
          "Content-Type": "application/json",
        },
        body: JSON.stringify(orderData),
      })

      if (response.ok) {
        const result = await response.json()
        toast({
          title: "Order Submitted Successfully!",
          description: `Order #${result.order.id} submitted${result.immediate_executions > 0 ? ` with ${result.immediate_executions} immediate execution(s)` : ""}`,
        })

        setFormData({
          symbol: "AAPL",
          side: "BUY",
          type: "LIMIT",
          price: "",
          quantity: "",
        })

        fetchOrders()
      } else {
        const error = await response.json()
        toast({
          title: "Order Failed",
          description: error.error || "Failed to submit order",
          variant: "destructive",
        })
      }
    } catch (error) {
      toast({
        title: "Network Error",
        description: "Please check your connection and try again",
        variant: "destructive",
      })
    } finally {
      setSubmitting(false)
    }
  }

  const handleCancel = async (orderId: number) => {
    try {
      const response = await fetch(`http://localhost:18080/orders/${orderId}/cancel`, {
        method: "POST",
      })

      if (response.ok) {
        toast({
          title: "Order Cancelled",
          description: `Order #${orderId} cancelled successfully`,
        })
        fetchOrders()
      } else {
        const error = await response.json()
        toast({
          title: "Cancellation Failed",
          description: error.error || "Failed to cancel order",
          variant: "destructive",
        })
      }
    } catch (error) {
      toast({
        title: "Network Error",
        description: "Please check your connection and try again",
        variant: "destructive",
      })
    }
  }

  return (
    <div className="space-y-6">
      {/* Header */}
      <motion.div initial={{ opacity: 0, y: -20 }} animate={{ opacity: 1, y: 0 }}>
        <div className="flex items-center justify-between">
          <div>
            <h1 className="text-2xl font-semibold text-gray-900">Order Management</h1>
            <p className="text-gray-600 mt-1">Place and manage your trading orders</p>
          </div>
          <Badge className="bg-blue-50 text-blue-700 border-blue-200 px-4 py-2">{orders.length} Active Orders</Badge>
        </div>
      </motion.div>

      <div className="grid gap-6 lg:grid-cols-3">
        {/* Order Form */}
        <motion.div initial={{ opacity: 0, x: -20 }} animate={{ opacity: 1, x: 0 }}>
          <Card className="border-gray-200">
            <CardHeader>
              <CardTitle className="flex items-center gap-2 text-gray-900">
                <Plus className="h-5 w-5 text-blue-600" />
                Place New Order
              </CardTitle>
            </CardHeader>
            <CardContent>
              <form onSubmit={handleSubmit} className="space-y-4">
                <div className="grid grid-cols-2 gap-4">
                  <div>
                    <Label htmlFor="symbol" className="text-gray-700">
                      Symbol
                    </Label>
                    <Input
                      id="symbol"
                      value={formData.symbol}
                      onChange={(e) => setFormData({ ...formData, symbol: e.target.value })}
                      placeholder="AAPL"
                      className="border-gray-200 focus:border-blue-300 focus:ring-2 focus:ring-blue-100"
                      required
                    />
                  </div>
                  <div>
                    <Label htmlFor="side" className="text-gray-700">
                      Side
                    </Label>
                    <Select value={formData.side} onValueChange={(value) => setFormData({ ...formData, side: value })}>
                      <SelectTrigger className="border-gray-200 focus:border-blue-300">
                        <SelectValue />
                      </SelectTrigger>
                      <SelectContent>
                        <SelectItem value="BUY" className="text-green-600">
                          <div className="flex items-center gap-2">
                            <TrendingUp className="h-4 w-4" />
                            Buy
                          </div>
                        </SelectItem>
                        <SelectItem value="SELL" className="text-red-600">
                          <div className="flex items-center gap-2">
                            <TrendingDown className="h-4 w-4" />
                            Sell
                          </div>
                        </SelectItem>
                      </SelectContent>
                    </Select>
                  </div>
                </div>

                <div className="grid grid-cols-2 gap-4">
                  <div>
                    <Label htmlFor="type" className="text-gray-700">
                      Order Type
                    </Label>
                    <Select value={formData.type} onValueChange={(value) => setFormData({ ...formData, type: value })}>
                      <SelectTrigger className="border-gray-200 focus:border-blue-300">
                        <SelectValue />
                      </SelectTrigger>
                      <SelectContent>
                        <SelectItem value="LIMIT">Limit Order</SelectItem>
                        <SelectItem value="MARKET">Market Order</SelectItem>
                      </SelectContent>
                    </Select>
                  </div>
                  <div>
                    <Label htmlFor="quantity" className="text-gray-700">
                      Quantity
                    </Label>
                    <Input
                      id="quantity"
                      type="number"
                      value={formData.quantity}
                      onChange={(e) => setFormData({ ...formData, quantity: e.target.value })}
                      placeholder="100"
                      min="1"
                      className="border-gray-200 focus:border-blue-300 focus:ring-2 focus:ring-blue-100"
                      required
                    />
                  </div>
                </div>

                {formData.type === "LIMIT" && (
                  <motion.div
                    initial={{ opacity: 0, height: 0 }}
                    animate={{ opacity: 1, height: "auto" }}
                    exit={{ opacity: 0, height: 0 }}
                  >
                    <Label htmlFor="price" className="text-gray-700">
                      Price ($)
                    </Label>
                    <Input
                      id="price"
                      type="number"
                      step="0.01"
                      value={formData.price}
                      onChange={(e) => setFormData({ ...formData, price: e.target.value })}
                      placeholder="150.00"
                      min="0.01"
                      className="border-gray-200 focus:border-blue-300 focus:ring-2 focus:ring-blue-100"
                      required
                    />
                  </motion.div>
                )}

                <Button
                  type="submit"
                  className="w-full bg-blue-600 hover:bg-blue-700 text-white font-medium py-2"
                  disabled={submitting}
                >
                  {submitting ? <Loader2 className="h-4 w-4 mr-2 animate-spin" /> : <Plus className="h-4 w-4 mr-2" />}
                  {submitting ? "Submitting..." : "Submit Order"}
                </Button>
              </form>
            </CardContent>
          </Card>
        </motion.div>

        {/* Active Orders */}
        <motion.div initial={{ opacity: 0, x: 20 }} animate={{ opacity: 1, x: 0 }} className="lg:col-span-2">
          <Card className="border-gray-200">
            <CardHeader>
              <CardTitle className="flex items-center gap-2 text-gray-900">
                <BookOpen className="h-5 w-5 text-blue-600" />
                Active Orders ({orders.length})
              </CardTitle>
            </CardHeader>
            <CardContent>
              {loading ? (
                <div className="space-y-3">
                  {[...Array(8)].map((_, i) => (
                    <div key={i} className="h-16 bg-gray-100 animate-pulse rounded-lg" />
                  ))}
                </div>
              ) : orders.length === 0 ? (
                <div className="text-center text-gray-500 py-12">
                  <BookOpen className="h-12 w-12 mx-auto mb-4 opacity-50" />
                  <p>No active orders</p>
                </div>
              ) : (
                <div className="space-y-3 max-h-96 overflow-y-auto">
                  <AnimatePresence>
                    {orders.slice(0, 15).map((order, index) => (
                      <motion.div
                        key={order.id}
                        initial={{ opacity: 0, y: 20 }}
                        animate={{ opacity: 1, y: 0 }}
                        exit={{ opacity: 0, y: -20 }}
                        transition={{ delay: index * 0.05 }}
                        className="flex justify-between items-center p-4 border border-gray-100 rounded-lg hover:border-gray-200 transition-all duration-200 hover:bg-gray-50"
                      >
                        <div className="flex items-center gap-4">
                          <div className={`p-2 rounded-lg ${order.side === "BUY" ? "bg-green-50" : "bg-red-50"}`}>
                            {order.side === "BUY" ? (
                              <TrendingUp className="h-4 w-4 text-green-600" />
                            ) : (
                              <TrendingDown className="h-4 w-4 text-red-600" />
                            )}
                          </div>
                          <div>
                            <div className="flex items-center gap-2">
                              <Badge
                                className={
                                  order.side === "BUY"
                                    ? "bg-green-50 text-green-700 border-green-200"
                                    : "bg-red-50 text-red-700 border-red-200"
                                }
                              >
                                {order.side}
                              </Badge>
                              <span className="font-medium text-gray-900">{order.symbol}</span>
                              <span className="text-sm text-gray-500">#{order.id}</span>
                            </div>
                            <div className="text-sm text-gray-600 mt-1">
                              {order.quantity.toLocaleString()} @ ${order.price.toFixed(2)}
                            </div>
                          </div>
                        </div>
                        <Button
                          variant="outline"
                          size="sm"
                          onClick={() => handleCancel(order.id)}
                          className="border-gray-200 hover:border-red-300 hover:bg-red-50 hover:text-red-600"
                        >
                          <X className="h-4 w-4" />
                        </Button>
                      </motion.div>
                    ))}
                  </AnimatePresence>
                </div>
              )}
            </CardContent>
          </Card>
        </motion.div>
      </div>

      {/* Order History Table */}
      <motion.div initial={{ opacity: 0, y: 20 }} animate={{ opacity: 1, y: 0 }} transition={{ delay: 0.3 }}>
        <Card className="border-gray-200">
          <CardHeader>
            <CardTitle className="text-gray-900">Order History</CardTitle>
          </CardHeader>
          <CardContent>
            <div className="overflow-x-auto">
              <Table>
                <TableHeader>
                  <TableRow className="border-gray-200">
                    <TableHead className="text-gray-600">Order ID</TableHead>
                    <TableHead className="text-gray-600">Symbol</TableHead>
                    <TableHead className="text-gray-600">Side</TableHead>
                    <TableHead className="text-gray-600">Type</TableHead>
                    <TableHead className="text-gray-600">Price</TableHead>
                    <TableHead className="text-gray-600">Quantity</TableHead>
                    <TableHead className="text-gray-600">Status</TableHead>
                    <TableHead className="text-gray-600">Time</TableHead>
                  </TableRow>
                </TableHeader>
                <TableBody>
                  {orders.slice(0, 20).map((order) => (
                    <TableRow key={order.id} className="border-gray-100 hover:bg-gray-50">
                      <TableCell className="text-gray-700">#{order.id}</TableCell>
                      <TableCell className="font-medium text-gray-900">{order.symbol}</TableCell>
                      <TableCell>
                        <Badge
                          className={
                            order.side === "BUY"
                              ? "bg-green-50 text-green-700 border-green-200"
                              : "bg-red-50 text-red-700 border-red-200"
                          }
                        >
                          {order.side}
                        </Badge>
                      </TableCell>
                      <TableCell className="text-gray-700">{order.type}</TableCell>
                      <TableCell className="font-mono text-gray-700">${order.price.toFixed(2)}</TableCell>
                      <TableCell className="text-gray-700">{order.quantity.toLocaleString()}</TableCell>
                      <TableCell>
                        <Badge className="bg-blue-50 text-blue-700 border-blue-200">{order.status}</Badge>
                      </TableCell>
                      <TableCell className="text-gray-500 text-sm">
                        {new Date(order.timestamp).toLocaleTimeString()}
                      </TableCell>
                    </TableRow>
                  ))}
                </TableBody>
              </Table>
            </div>
          </CardContent>
        </Card>
      </motion.div>
    </div>
  )
}
